#include "llvm_gen.h"
#include <stdlib.h>
#include <string.h>

void llvm_gen_init(LLVMGen* gen, const char* filename) {
    gen->output_file = fopen(filename, "w");
    gen->register_counter = 1; // LLVM local registers start at %1
    
    if (!gen->output_file) {
        fprintf(stderr, "LLVM Gen Error: Could not create output file %s\n", filename);
        exit(1);
    }
    
    // Write the LLVM Module header
    fprintf(gen->output_file, "; ModuleID = 'VoLangCore'\n");
    fprintf(gen->output_file, "source_filename = \"volang_script\"\n\n");
    
    // Define the main entry point for the OS
    fprintf(gen->output_file, "define i64 @main() {\n");
    fprintf(gen->output_file, "entry:\n");
}

// Recursively evaluate expressions and return the CPU register holding the result
static int generate_expression(LLVMGen* gen, AstNode* node) {
    if (node->type == AST_INTEGER_LITERAL) {
        int reg = gen->register_counter++;
        // Load the number into a new register
        fprintf(gen->output_file, "  %%%d = add i64 0, %lld\n", reg, node->data.int_literal.value);
        return reg;
    }
    
    if (node->type == AST_IDENTIFIER) {
        int reg = gen->register_counter++;
        // Load a variable from memory (ptr) into a register
        fprintf(gen->output_file, "  %%%d = load i64, ptr %%%.*s\n", 
                reg, (int)node->data.ident.length, node->data.ident.value);
        return reg;
    }
    
    if (node->type == AST_INFIX_EXPRESSION) {
        int left_reg = generate_expression(gen, node->data.infix.left);
        int right_reg = generate_expression(gen, node->data.infix.right);
        int res_reg = gen->register_counter++;
        
        if (node->data.infix.operator_str[0] == '+') {
            fprintf(gen->output_file, "  %%%d = add i64 %%%d, %%%d\n", res_reg, left_reg, right_reg);
        } else if (node->data.infix.operator_str[0] == '-') {
            fprintf(gen->output_file, "  %%%d = sub i64 %%%d, %%%d\n", res_reg, left_reg, right_reg);
        }
        return res_reg;
    }
    
    return 0;
}

// Generate LLVM instructions for statements
static void generate_statement(LLVMGen* gen, AstNode* node) {
    if (!node) return;

    if (node->type == AST_LET_STATEMENT) {
        // 1. Allocate memory on the stack for the variable (64-bit int)
        fprintf(gen->output_file, "  %%%.*s = alloca i64\n", 
                (int)node->data.let_stmt.name->length, node->data.let_stmt.name->value);
        
        // 2. Evaluate the expression into a register
        int val_reg = generate_expression(gen, node->data.let_stmt.value);
        
        // 3. Store the register's value into the allocated memory
        fprintf(gen->output_file, "  store i64 %%%d, ptr %%%.*s\n", 
                val_reg, (int)node->data.let_stmt.name->length, node->data.let_stmt.name->value);
                
    } else if (node->type == AST_RETURN_STATEMENT) {
        int val_reg = generate_expression(gen, node->data.return_stmt.return_value);
        fprintf(gen->output_file, "  ret i64 %%%d\n", val_reg);
        
    } else if (node->type == AST_EXPRESSION_STATEMENT) {
        generate_expression(gen, node->data.expression_stmt);
    }
}

int llvm_gen_program(LLVMGen* gen, Program* program) {
    for (size_t i = 0; i < program->statement_count; i++) {
        generate_statement(gen, program->statements[i]);
    }
    
    // Safety fallback: if the user code didn't have a 'return', we return 0 automatically
    if (program->statement_count > 0 && 
        program->statements[program->statement_count - 1]->type != AST_RETURN_STATEMENT) {
        fprintf(gen->output_file, "  ret i64 0\n");
    }
    
    return 1;
}

void llvm_gen_close(LLVMGen* gen) {
    fprintf(gen->output_file, "}\n");
    fclose(gen->output_file);
}
