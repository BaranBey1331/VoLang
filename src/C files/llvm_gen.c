#include "llvm_gen.h"
#include <stdlib.h>
#include <string.h>

void llvm_gen_init(LLVMGen* gen, const char* filename) {
    gen->output_file = fopen(filename, "w");
    gen->register_counter = 1; 
    
    if (!gen->output_file) {
        fprintf(stderr, "LLVM Gen Error: Could not create output file %s\n", filename);
        exit(1);
    }
    
    fprintf(gen->output_file, "; ModuleID = 'VoLangCore'\n");
    fprintf(gen->output_file, "source_filename = \"volang_script\"\n\n");
}

static int generate_expression(LLVMGen* gen, AstNode* node) {
    if (node->type == AST_INTEGER_LITERAL) {
        int reg = gen->register_counter++;
        fprintf(gen->output_file, "  %%%d = add i64 0, %lld\n", reg, node->data.int_literal.value);
        return reg;
    }
    
    if (node->type == AST_IDENTIFIER) {
        int reg = gen->register_counter++;
        // Load local/global variable from memory
        fprintf(gen->output_file, "  %%%d = load i64, ptr %%%.*s\n", 
                reg, (int)node->data.ident.length, node->data.ident.value);
        return reg;
    }
    
    if (node->type == AST_FUNCTION_CALL) {
        // Evaluate arguments
        int arg_regs[8];
        for (size_t i = 0; i < node->data.function_call.arg_count; i++) {
            arg_regs[i] = generate_expression(gen, node->data.function_call.arguments[i]);
        }
        
        int res_reg = gen->register_counter++;
        fprintf(gen->output_file, "  %%%d = call i64 @%.*s(", 
                res_reg, (int)node->data.function_call.function_name->length, node->data.function_call.function_name->value);
                
        for (size_t i = 0; i < node->data.function_call.arg_count; i++) {
            fprintf(gen->output_file, "i64 %%%d", arg_regs[i]);
            if (i < node->data.function_call.arg_count - 1) fprintf(gen->output_file, ", ");
        }
        fprintf(gen->output_file, ")\n");
        return res_reg;
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

static void generate_statement(LLVMGen* gen, AstNode* node) {
    if (!node) return;

    if (node->type == AST_LET_STATEMENT) {
        fprintf(gen->output_file, "  %%%.*s = alloca i64\n", 
                (int)node->data.let_stmt.name->length, node->data.let_stmt.name->value);
        
        int val_reg = generate_expression(gen, node->data.let_stmt.value);
        
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
    // 1. First Pass: Generate user-defined functions (fn add(x,y) {...})
    for (size_t i = 0; i < program->statement_count; i++) {
        AstNode* stmt = program->statements[i];
        if (stmt->type == AST_FUNCTION) {
            gen->register_counter = 1; // Reset register counter for new function
            
            fprintf(gen->output_file, "define i64 @%.*s(", 
                    (int)stmt->data.function_stmt.name->length, stmt->data.function_stmt.name->value);
            
            // Define parameters
            for (size_t p = 0; p < stmt->data.function_stmt.param_count; p++) {
                fprintf(gen->output_file, "i64 %%arg_%.*s", 
                        (int)stmt->data.function_stmt.parameters[p]->length, stmt->data.function_stmt.parameters[p]->value);
                if (p < stmt->data.function_stmt.param_count - 1) fprintf(gen->output_file, ", ");
            }
            fprintf(gen->output_file, ") {\nentry:\n");
            
            // FIX: Allocate memory for parameters so 'load i64, ptr %x' works seamlessly!
            for (size_t p = 0; p < stmt->data.function_stmt.param_count; p++) {
                fprintf(gen->output_file, "  %%%.*s = alloca i64\n", 
                        (int)stmt->data.function_stmt.parameters[p]->length, stmt->data.function_stmt.parameters[p]->value);
                fprintf(gen->output_file, "  store i64 %%arg_%.*s, ptr %%%.*s\n", 
                        (int)stmt->data.function_stmt.parameters[p]->length, stmt->data.function_stmt.parameters[p]->value,
                        (int)stmt->data.function_stmt.parameters[p]->length, stmt->data.function_stmt.parameters[p]->value);
            }
            
            // Generate body
            for (size_t b = 0; b < stmt->data.function_stmt.body_count; b++) {
                generate_statement(gen, stmt->data.function_stmt.body[b]);
            }
            
            // Fallback return if user forgot
            if (stmt->data.function_stmt.body_count == 0 || 
                stmt->data.function_stmt.body[stmt->data.function_stmt.body_count - 1]->type != AST_RETURN_STATEMENT) {
                fprintf(gen->output_file, "  ret i64 0\n");
            }
            fprintf(gen->output_file, "}\n\n");
        }
    }

    // 2. Second Pass: Generate the main entry point for loose statements (let result = add(...))
    gen->register_counter = 1;
    fprintf(gen->output_file, "define i64 @main() {\nentry:\n");
    
    for (size_t i = 0; i < program->statement_count; i++) {
        AstNode* stmt = program->statements[i];
        if (stmt->type != AST_FUNCTION) {
            generate_statement(gen, stmt);
        }
    }
    
    // Main fallback return
    fprintf(gen->output_file, "  ret i64 0\n}\n");
    return 1;
}

void llvm_gen_close(LLVMGen* gen) {
    fclose(gen->output_file);
}
