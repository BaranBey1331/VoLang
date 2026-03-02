#include "compiler.h"
#include <stdlib.h>
#include <stdio.h>

// Helper to emit a single byte instruction
static void emit(Compiler* compiler, uint8_t op) {
    if (compiler->bytecode.length >= compiler->bytecode.capacity) {
        compiler->bytecode.capacity *= 2;
        compiler->bytecode.instructions = (uint8_t*)realloc(
            compiler->bytecode.instructions, 
            compiler->bytecode.capacity
        );
    }
    compiler->bytecode.instructions[compiler->bytecode.length++] = op;
}

Compiler* compiler_create() {
    Compiler* c = (Compiler*)malloc(sizeof(Compiler));
    c->bytecode.capacity = 256;
    c->bytecode.length = 0;
    c->bytecode.instructions = (uint8_t*)malloc(c->bytecode.capacity);
    c->symbol_table = symbol_table_create();
    return c;
}

// Recursive function to walk the AST and emit opcodes
static int compile_node(Compiler* compiler, AstNode* node) {
    if (!node) return 1;

    switch (node->type) {
        case AST_LET_STATEMENT:
            // 1. Compile the right side of the equals sign (the value)
            compile_node(compiler, node->data.let_stmt.value);
            // 2. Define it in the symbol table
            symbol_table_define(
                compiler->symbol_table, 
                node->data.let_stmt.name->value, 
                node->data.let_stmt.name->length
            );
            // 3. Emit the SET instruction
            emit(compiler, OP_SET_GLOBAL);
            break;

        case AST_INFIX_EXPRESSION:
            // Post-order traversal: Left, Right, Operator (Standard stack machine logic)
            compile_node(compiler, node->data.infix.left);
            compile_node(compiler, node->data.infix.right);
            
            if (node->data.infix.operator_str[0] == '+') {
                emit(compiler, OP_ADD);
            } else if (node->data.infix.operator_str[0] == '-') {
                emit(compiler, OP_SUBTRACT);
            }
            break;

        case AST_INTEGER_LITERAL:
            // For now, emit a constant instruction
            emit(compiler, OP_CONSTANT);
            // In a real VM, we would emit the index of the constant pool here
            // emit(compiler, value_index); 
            break;

        case AST_RETURN_STATEMENT:
            compile_node(compiler, node->data.return_stmt.return_value);
            emit(compiler, OP_RETURN);
            break;

        case AST_EXPRESSION_STATEMENT:
            compile_node(compiler, node->data.expression_stmt);
            break;

        case AST_IDENTIFIER:
            // When we see a variable name, we want to load it
            emit(compiler, OP_GET_GLOBAL);
            break;
            
        default:
            break; // Skip unhandled nodes for now
    }
    return 1;
}

int compile_program(Compiler* compiler, Program* program) {
    for (size_t i = 0; i < program->statement_count; i++) {
        compile_node(compiler, program->statements[i]);
    }
    return 1; // Success
}

void compiler_free(Compiler* compiler) {
    free(compiler->bytecode.instructions);
    symbol_table_free(compiler->symbol_table);
    free(compiler);
}
