#ifndef AST_H
#define AST_H

#include "token.h"
#include <stddef.h>
#include <stdint.h>

/* --- Arena Allocator for Ultra-Fast AST Generation --- */
typedef struct {
    uint8_t* memory;
    size_t capacity;
    size_t offset;
} Arena;

void arena_init(Arena* arena, size_t size);
void* arena_alloc(Arena* arena, size_t size);
void arena_free(Arena* arena);


/* --- Abstract Syntax Tree (AST) Nodes --- */
typedef enum {
    AST_PROGRAM,
    AST_LET_STATEMENT,
    AST_RETURN_STATEMENT,
    AST_EXPRESSION_STATEMENT,
    AST_IDENTIFIER,
    AST_INTEGER_LITERAL,
    AST_INFIX_EXPRESSION
} AstNodeType;

typedef struct AstNode AstNode;

typedef struct {
    Token token;
    const char* value;
    size_t length;
} Identifier;

typedef struct {
    Token token;
    Identifier* name;
    AstNode* value; 
} LetStatement;

typedef struct {
    Token token; 
    AstNode* return_value;
} ReturnStatement;

typedef struct {
    Token token; 
    AstNode* left;
    const char* operator_str;
    size_t operator_len;
    AstNode* right;
} InfixExpression;

typedef struct {
    Token token;
    long long value;
} IntegerLiteral;

struct AstNode {
    AstNodeType type;
    union {
        LetStatement let_stmt;
        ReturnStatement return_stmt;
        Identifier ident;
        IntegerLiteral int_literal;
        InfixExpression infix;
        AstNode* expression_stmt; 
    } data;
};

typedef struct {
    AstNode** statements; 
    size_t statement_count;
    size_t statement_capacity;
} Program;

Program* create_program(Arena* arena);
void program_add_statement(Arena* arena, Program* program, AstNode* stmt);
void print_ast(AstNode* node, int indentation);

#endif // AST_H
