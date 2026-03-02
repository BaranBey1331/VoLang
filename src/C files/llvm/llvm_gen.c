#include "llvm_gen.h"
#include <stdlib.h>
#include <string.h>

void llvm_gen_init(LLVMGen* gen, const char* filename) {
    gen->output_file = fopen(filename, "w");
    gen->register_counter = 1; 
    gen->current_scope_depth = 0;
    gen->scope_var_counts[0] = 0; // Initialize Global scope
    
    if (!gen->output_file) {
        fprintf(stderr, "LLVM Gen Error: Could not create output file %s\n", filename);
        exit(1);
    }
    
    fprintf(gen->output_file, "; ModuleID = 'VoLangCore'\n");
    fprintf(gen->output_file, "source_filename = \"volang_script\"\n\n");
    fprintf(gen->output_file, "; --- Built-in Native Functions ---\n");
    fprintf(gen->output_file, "declare i32 @printf(ptr, ...)\n");
    fprintf(gen->output_file, "@.str.int = private unnamed_addr constant [6 x i8] c\"%%lld\\0A\\00\"\n\n");
}

static void enter_scope(LLVMGen* gen) {
    if (gen->current_scope_depth < MAX_SCOPE_DEPTH - 1) {
        gen->current_scope_depth++;
        gen->scope_var_counts[gen->current_scope_depth] = 0;
    }
}

static void exit_scope(LLVMGen* gen) {
    if (gen->current_scope_depth > 0) {
        gen->current_scope_depth--;
    }
}

// Returns depth level. 0 = Global (@), >0 = Local (%)
static int resolve_scope(LLVMGen* gen, const char* name, size_t len) {
    for (int d = gen->current_scope_depth; d >= 0; d--) {
        for (int i = gen->scope_var_counts[d] - 1; i >= 0; i--) {
            if (gen->scope_var_lengths[d][i] == len && strncmp(gen->scope_vars[d][i], name, len) == 0) {
                return d; 
            }
        }
    }
    return 1; // Default to local if undeclared (prevents total crash, allows LLVM to throw error)
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
        int depth = resolve_scope(gen, node->data.ident.value, node->data.ident.length);
        char prefix = (depth == 0) ? '@' : '%';
        
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
        
        if (node->data.function_call.function_name->length == 5 && 
            strncmp(node->data.function_call.function_name->value, "print", 5) == 0) {
            
            int res_reg = gen->register_counter++;
            fprintf(gen->output_file, "  %%%d = call i32 (ptr, ...) @printf(ptr @.str.int, i64 %%%d)\n", 
                    res_reg, arg_regs[0]);
            int cast_reg = gen->register_counter++;
            fprintf(gen->output_file, "  %%%d = sext i32 %%%d to i64\n", cast_reg, res_reg);
            return cast_reg;
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
        
        char op = node->data.infix.operator_str[0];
        
        if (op == '+') {
            fprintf(gen->output_file, "  %%%d = add i64 %%%d, %%%d\n", res_reg, left_reg, right_reg);
        } else if (op == '-') {
            fprintf(gen->output_file, "  %%%d = sub i64 %%%d, %%%d\n", res_reg, left_reg, right_reg);
        } else if (op == '*') {
            fprintf(gen->output_file, "  %%%d = mul i64 %%%d, %%%d\n", res_reg, left_reg, right_reg);
        } else if (op == '/') {
            fprintf(gen->output_file, "  %%%d = sdiv i64 %%%d, %%%d\n", res_reg, left_reg, right_reg);
        } else if (op == '%') {
            fprintf(gen->output_file, "  %%%d = srem i64 %%%d, %%%d\n", res_reg, left_reg, right_reg);
        }
        return res_reg;
    }
    
    return 0;
}

static void generate_statement(LLVMGen* gen, AstNode* node) {
    if (!node) return;

    if (node->type == AST_LET_STATEMENT) {
        int val_reg = generate_expression(gen, node->data.let_stmt.value);
        
        if (gen->current_scope_depth > 0) {
            // Register local variable in current scope
            if (gen->scope_var_counts[gen->current_scope_depth] < MAX_VARS_PER_SCOPE) {
                size_t idx = gen->scope_var_counts[gen->current_scope_depth]++;
                gen->scope_vars[gen->current_scope_depth][idx] = node->data.let_stmt.name->value;
                gen->scope_var_lengths[gen->current_scope_depth][idx] = node->data.let_stmt.name->length;
            }
            
            fprintf(gen->output_file, "  %%%.*s = alloca i64\n", 
                    (int)node->data.let_stmt.name->length, node->data.let_stmt.name->value);
            fprintf(gen->output_file, "  store i64 %%%d, ptr %%%.*s\n", 
                    val_reg, (int)node->data.let_stmt.name->length, node->data.let_stmt.name->value);
        } else {
            // Global store
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
    gen->current_scope_depth = 0; // Global Scope

    // Pass 0: Register global variables
    for (size_t i = 0; i < program->statement_count; i++) {
        AstNode* stmt = program->statements[i];
        if (stmt->type == AST_LET_STATEMENT) {
            if (gen->scope_var_counts[0] < MAX_VARS_PER_SCOPE) {
                size_t idx = gen->scope_var_counts[0]++;
                gen->scope_vars[0][idx] = stmt->data.let_stmt.name->value;
                gen->scope_var_lengths[0][idx] = stmt->data.let_stmt.name->length;
            }
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
            enter_scope(gen); // Entering function body (Depth 1)
            
            fprintf(gen->output_file, "define i64 @%.*s(", 
                    (int)stmt->data.function_stmt.name->length, stmt->data.function_stmt.name->value);
            
            for (size_t p = 0; p < stmt->data.function_stmt.param_count; p++) {
                if (gen->scope_var_counts[gen->current_scope_depth] < MAX_VARS_PER_SCOPE) {
                    size_t idx = gen->scope_var_counts[gen->current_scope_depth]++;
                    gen->scope_vars[gen->current_scope_depth][idx] = stmt->data.function_stmt.parameters[p]->value;
                    gen->scope_var_lengths[gen->current_scope_depth][idx] = stmt->data.function_stmt.parameters[p]->length;
                }
                
                fprintf(gen->output_file, "i64 %%arg_%.*s", 
                        (int)stmt->data.function_stmt.parameters[p]->length, stmt->data.function_stmt.parameters[p]->value);
                if (p < stmt->data.function_stmt.param_count - 1) fprintf(gen->output_file, ", ");
            }
            fprintf(gen->output_file, ") {\nentry:\n");
            
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
            
            exit_scope(gen); // Back to Global Scope
        }
    }

    // Pass 2: Generate the main execution block
    gen->register_counter = 1;
    enter_scope(gen); // Enter Main body (Depth 1)
    
    fprintf(gen->output_file, "define i64 @main() {\nentry:\n");
    
    for (size_t i = 0; i < program->statement_count; i++) {
        AstNode* stmt = program->statements[i];
        if (stmt->type != AST_FUNCTION && stmt->type != AST_LET_STATEMENT) {
            generate_statement(gen, stmt);
        } else if (stmt->type == AST_LET_STATEMENT) {
            // Globals were already allocated, just store values
            int val_reg = generate_expression(gen, stmt->data.let_stmt.value);
            fprintf(gen->output_file, "  store i64 %%%d, ptr @%.*s\n", 
                    val_reg, (int)stmt->data.let_stmt.name->length, stmt->data.let_stmt.name->value);
        }
    }
    
    fprintf(gen->output_file, "  ret i64 0\n}\n");
    exit_scope(gen); // Clean exit

    return 1;
}

void llvm_gen_close(LLVMGen* gen) {
    fclose(gen->output_file);
}
