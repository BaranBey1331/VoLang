#ifndef LLVM_GEN_H
#define LLVM_GEN_H

#include "ast.h"
#include <stdio.h>
#include <stdbool.h>

#define MAX_SCOPE_DEPTH 64
#define MAX_VARS_PER_SCOPE 256

// LLVM Generator State
typedef struct {
    FILE* output_file;
    int register_counter; 
    
    // Multi-level Scope Tracking
    int current_scope_depth;
    const char* scope_vars[MAX_SCOPE_DEPTH][MAX_VARS_PER_SCOPE];
    size_t scope_var_lengths[MAX_SCOPE_DEPTH][MAX_VARS_PER_SCOPE];
    size_t scope_var_counts[MAX_SCOPE_DEPTH];
    
} LLVMGen;

// Initialize the LLVM generator
void llvm_gen_init(LLVMGen* gen, const char* filename);

// Generate pure LLVM IR from the AST
int llvm_gen_program(LLVMGen* gen, Program* program);

// Close the file
void llvm_gen_close(LLVMGen* gen);

#endif // LLVM_GEN_H
