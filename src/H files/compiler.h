#ifndef COMPILER_H
#define COMPILER_H

#include "ast.h"
#include "opcode.h"
#include "symbol_table.h"
#include <stdint.h>

// Represents a sequence of compiled bytecode instructions
typedef struct {
    uint8_t* instructions;
    int length;
    int capacity;
} Bytecode;

// The Compiler state
typedef struct {
    Bytecode bytecode;
    SymbolTable* symbol_table;
} Compiler;

// Initialize the compiler
Compiler* compiler_create();

// Compile the AST Program into Bytecode
int compile_program(Compiler* compiler, Program* program);

// Free compiler memory
void compiler_free(Compiler* compiler);

#endif // COMPILER_H
