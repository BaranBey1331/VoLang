#ifndef LLVM_GEN_H
#define LLVM_GEN_H

#include "ast.h"
#include <stdio.h>

// LLVM Generator State
typedef struct {
    FILE* output_file;
    int register_counter; // Tracks the %1, %2 CPU registers in LLVM
} LLVMGen;

// Initialize the LLVM generator
void llvm_gen_init(LLVMGen* gen, const char* filename);

// Generate pure LLVM IR from the AST
int llvm_gen_program(LLVMGen* gen, Program* program);

// Close the file
void llvm_gen_close(LLVMGen* gen);

#endif // LLVM_GEN_H
