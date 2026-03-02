#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include <stdio.h>

// Code Generator State
typedef struct {
    FILE* output_file;
    int indentation;
} CodeGen;

// Initialize the code generator with an output file
void codegen_init(CodeGen* cg, const char* output_filename);

// Generate C code from the AST Program
int codegen_program(CodeGen* cg, Program* program);

// Close the file and cleanup
void codegen_close(CodeGen* cg);

#endif // CODEGEN_H
