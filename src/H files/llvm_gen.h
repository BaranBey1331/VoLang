#ifndef LLVM_GEN_H
#define LLVM_GEN_H

#include "ast.h"
#include <stdio.h>
#include <stdbool.h>

#define MAX_SCOPE_VARS 1024

// LLVM Generator State
typedef struct {
    FILE* output_file;
    int register_counter; 
    
    // Fast Scope Tracking for 1M LOC/s (Zero malloc)
    bool in_function;
    
    const char* globals[MAX_SCOPE_VARS];
    size_t global_lengths[MAX_SCOPE_VARS];
    size_t global_count;
    
    const char* locals[MAX_SCOPE_VARS];
    size_t local_lengths[MAX_SCOPE_VARS];
    size_t local_count;
    
} LLVMGen;

// Initialize the LLVM generator
void llvm_gen_init(LLVMGen* gen, const char* filename);

// Generate pure LLVM IR from the AST
int llvm_gen_program(LLVMGen* gen, Program* program);

// Close the file
void llvm_gen_close(LLVMGen* gen);

#endif // LLVM_GEN_H
