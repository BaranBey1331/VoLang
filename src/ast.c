#include "ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* --- Arena Allocator Implementation --- */
void arena_init(Arena* arena, size_t size) {
    arena->memory = (uint8_t*)malloc(size);
    arena->capacity = size;
    arena->offset = 0;
}

void* arena_alloc(Arena* arena, size_t size) {
    size_t aligned_size = (size + 7) & ~7; 
    
    if (arena->offset + aligned_size > arena->capacity) {
        fprintf(stderr, "COMPILER ERROR: Arena allocator Out Of Memory!\n");
        exit(1);
    }
    
    void* ptr = &arena->memory[arena->offset];
    arena->offset += aligned_size;
    return ptr;
}

void arena_free(Arena* arena) {
    free(arena->memory);
    arena->memory = NULL;
    arena->offset = 0;
    arena->capacity = 0;
}

/* --- AST Implementation --- */
Program* create_program(Arena* arena) {
    Program* prog = (Program*)arena_alloc(arena, sizeof(Program));
    prog->statement_capacity = 64; 
    prog->statement_count = 0;
    prog->statements = (AstNode**)arena_alloc(arena, sizeof(AstNode*) * prog->statement_capacity);
    return prog;
}

void program_add_statement(Arena* arena, Program* program, AstNode* stmt) {
    if (program->statement_count >= program->statement_capacity) {
        size_t new_cap = program->statement_capacity * 2;
        AstNode** new_stmts = (AstNode**)arena_alloc(arena, sizeof(AstNode*) * new_cap);
        memcpy(new_stmts, program->statements, sizeof(AstNode*) * program->statement_count);
        program->statements = new_stmts;
        program->statement_capacity = new_cap;
    }
    program->statements[program->statement_count++] = stmt;
}

void print_ast(AstNode* node, int indent) {
    if (!node) return;
    
    for (int i = 0; i < indent; i++) printf("  ");
    
    switch (node->type) {
        case AST_LET_STATEMENT:
            printf("LetStatement: %.*s =\n", 
                   (int)node->data.let_stmt.name->length, 
                   node->data.let_stmt.name->value);
            print_ast(node->data.let_stmt.value, indent + 1);
            break;
        case AST_RETURN_STATEMENT:
            printf("ReturnStatement:\n");
            print_ast(node->data.return_stmt.return_value, indent + 1);
            break;
        case AST_IDENTIFIER:
            printf("Identifier: %.*s\n", 
                   (int)node->data.ident.length, 
                   node->data.ident.value);
            break;
        case AST_INTEGER_LITERAL:
            printf("IntegerLiteral: %lld\n", node->data.int_literal.value);
            break;
        case AST_INFIX_EXPRESSION:
            printf("InfixExpression: %.*s\n", 
                   (int)node->data.infix.operator_len, 
                   node->data.infix.operator_str);
            print_ast(node->data.infix.left, indent + 1);
            print_ast(node->data.infix.right, indent + 1);
            break;
        case AST_EXPRESSION_STATEMENT:
            print_ast(node->data.expression_stmt, indent);
            break;
        default:
            printf("UNKNOWN_NODE\n");
    }
}
