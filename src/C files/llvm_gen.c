#include "llvm_gen.h"
#include <stdlib.h>
#include <string.h>

void llvm_gen_init(LLVMGen* gen, const char* filename) {
    gen->output_file = fopen(filename, "w");
    gen->register_counter = 1; 
    gen->in_function = false;
    gen->global_count = 0;
    gen->local_count = 0;
    
    if (!gen->output_file) {
        fprintf(stderr, "LLVM Gen Error: Could not create output file %s\n", filename);
        exit(1);
    }
    
    fprintf(gen->output_file, "; ModuleID = 'VoLangCore'\n");
    fprintf(gen->output_file, "source_filename = \"volang_script\"\n\n");
}

// Helper: Determine if a variable is local (%) or global (@)
// Returns 1 for Local, 2 for Global
static int resolve_scope(LLVMGen* gen, const char* name, size_t len) {
    // Check locals first (allows shadowing of globals)
    for (int i = gen->local_count - 1; i >= 0; i--) {
        if (gen->local_lengths[i] == len && strncmp(gen->locals[i], name, len) == 0) {
            return 1; 
        }
    }
    // Check globals
    for (int i = gen->global_count - 1; i >= 0; i--) {
        if (gen->global_lengths[i] == len && strncmp(gen->globals[i], name, len) == 0) {
            return 2;
        }
    }
    return 1; // Default to local if unknown (fallback)
}

static int generate_expression(LLVMGen* gen, AstNode* node) {
    if (!node) return 0;

    if (node->type == AST_INTEGER_LITERAL) {
        int reg = gen->register_counter++;
        fprintf(gen->output_file, "  %%%d = add i64 0, %lld\n", reg, node->data.int_literal.value);
        return reg;
    }
    
    if (node->type == AST_IDENTIFIER) {
        int reg = gen->register_counter++;
        int scope = resolve_scope(gen, node->data.ident.value, node->data.ident.length);
        char prefix = (scope == 2) ? '@' : '%';
        
        fprintf(gen->output_file, "  %%%d = load i64, ptr %c%.*s\n", 
                reg, prefix, (int)node->data.ident.length, node->data.ident.value);
        return reg;
    }
    
    if (node->type == AST_FUNCTION_CALL) {
        size_t arg_count = node->data.function_call.arg_count;
        int arg_regs[arg_count > 0 ? arg_count : 1]; 
        
        for (size_t i = 0; i < arg_count; i++) {
            arg_regs[i] = generate_expression(gen, node->data.function_call.arguments[i]);
        }
        
        int res_reg = gen->register_counter++;
        fprintf(gen->output_file, "  %%%d = call i64 @%.*s(", 
                res_reg, (int)node->data.function_call.function_name->length, node->data.function_call.function_name->value);
                
        for (size_t i = 0; i < arg_count; i++) {
            fprintf(gen->output_file, "i64 %%%d", arg_regs[i]);
            if (i < arg_count - 1) fprintf(gen->output_file, ", ");
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
        int val_reg = generate_expression(gen, node->data.let_stmt.value);
        
        if (gen->in_function) {
            // Local variable allocation
            if (gen->local_count < MAX_SCOPE_VARS) {
                gen->locals[gen->local_count] = node->data.let_stmt.name->value;
                gen->local_lengths[gen->local_count] = node->data.let_stmt.name->length;
                gen->local_count++;
            }
            
            fprintf(gen->output_file, "  %%%.*s = alloca i64\n", 
                    (int)node->data.let_stmt.name->length, node->data.let_stmt.name->value);
            fprintf(gen->output_file, "  store i64 %%%d, ptr %%%.*s\n", 
                    val_reg, (int)node->data.let_stmt.name->length, node->data.let_stmt.name->value);
        } else {
            // Global variable assignment (allocated in Pass 0)
            fprintf(gen->output_file, "  store i64 %%%d, ptr @%.*s\n", 
                    val_reg, (int)node->data.let_stmt.name->length, node->data.let_stmt.name->value);
        }
                
    } else if (node->type == AST_RETURN_STATEMENT) {
        int val_reg = generate_expression(gen, node->data.return_stmt.return_value);
        fprintf(gen->output_file, "  ret i64 %%%d\n", val_reg);
        
    } else if (node->type == AST_EXPRESSION_STATEMENT) {
        generate_expression(gen, node->data.expression_stmt);
    }
}

int llvm_gen_program(LLVMGen* gen, Program* program) {
    // Pass 0: Register all global variables
    for (size_t i = 0; i < program->statement_count; i++) {
        AstNode* stmt = program->statements[i];
        if (stmt->type == AST_LET_STATEMENT) {
            if (gen->global_count < MAX_SCOPE_VARS) {
                gen->globals[gen->global_count] = stmt->data.let_stmt.name->value;
                gen->global_lengths[gen->global_count] = stmt->data.let_stmt.name->length;
                gen->global_count++;
            }
            // Define global variable in LLVM IR
            fprintf(gen->output_file, "@%.*s = common global i64 0\n", 
                    (int)stmt->data.let_stmt.name->length, stmt->data.let_stmt.name->value);
        }
    }
    fprintf(gen->output_file, "\n");

    // Pass 1: Generate user-defined functions
    for (size_t i = 0; i < program->statement_count; i++) {
        AstNode* stmt = program->statements[i];
        if (stmt->type == AST_FUNCTION) {
            gen->register_counter = 1; 
            gen->in_function = true;
            gen->local_count = 0;
            
            fprintf(gen->output_file, "define i64 @%.*s(", 
                    (int)stmt->data.function_stmt.name->length, stmt->data.function_stmt.name->value);
            
            // Register parameters as local variables
            for (size_t p = 0; p < stmt->data.function_stmt.param_count; p++) {
                if (gen->local_count < MAX_SCOPE_VARS) {
                    gen->locals[gen->local_count] = stmt->data.function_stmt.parameters[p]->value;
                    gen->local_lengths[gen->local_count] = stmt->data.function_stmt.parameters[p]->length;
                    gen->local_count++;
                }
                
                fprintf(gen->output_file, "i64 %%arg_%.*s", 
                        (int)stmt->data.function_stmt.parameters[p]->length, stmt->data.function_stmt.parameters[p]->value);
                if (p < stmt->data.function_stmt.param_count - 1) fprintf(gen->output_file, ", ");
            }
            fprintf(gen->output_file, ") {\nentry:\n");
            
            // Allocate parameters on the local stack
            for (size_t p = 0; p < stmt->data.function_stmt.param_count; p++) {
                fprintf(gen->output_file, "  %%%.*s = alloca i64\n", 
                        (int)stmt->data.function_stmt.parameters[p]->length, stmt->data.function_stmt.parameters[p]->value);
                fprintf(gen->output_file, "  store i64 %%arg_%.*s, ptr %%%.*s\n", 
                        (int)stmt->data.function_stmt.parameters[p]->length, stmt->data.function_stmt.parameters[p]->value,
                        (int)stmt->data.function_stmt.parameters[p]->length, stmt->data.function_stmt.parameters[p]->value);
            }
            
            for (size_t b = 0; b < stmt->data.function_stmt.body_count; b++) {
                generate_statement(gen, stmt->data.function_stmt.body[b]);
            }
            
            if (stmt->data.function_stmt.body_count == 0 || 
                stmt->data.function_stmt.body[stmt->data.function_stmt.body_count - 1]->type != AST_RETURN_STATEMENT) {
                fprintf(gen->output_file, "  ret i64 0\n");
            }
            fprintf(gen->output_file, "}\n\n");
            
            gen->in_function = false; // Reset state
        }
    }

    // Pass 2: Generate the main execution block
    gen->register_counter = 1;
    gen->in_function = false;
    gen->local_count = 0; // Clear locals for main block
    
    fprintf(gen->output_file, "define i64 @main() {\nentry:\n");
    
    for (size_t i = 0; i < program->statement_count; i++) {
        AstNode* stmt = program->statements[i];
        if (stmt->type != AST_FUNCTION) {
            generate_statement(gen, stmt);
        }
    }
    
    fprintf(gen->output_file, "  ret i64 0\n}\n");
    return 1;
}

void llvm_gen_close(LLVMGen* gen) {
    fclose(gen->output_file);
}
