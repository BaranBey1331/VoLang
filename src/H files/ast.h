#ifndef AST_H
#define AST_H

#include "token.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t* memory;
    size_t capacity;
    size_t offset;
} Arena;

void arena_init(Arena* arena, size_t size);
void* arena_alloc(Arena* arena, size_t size);
void arena_free(Arena* arena);

typedef enum {
    AST_PROGRAM,
    AST_LET_STATEMENT,
    AST_RETURN_STATEMENT,
    AST_EXPRESSION_STATEMENT,
    AST_IDENTIFIER,
    AST_INTEGER_LITERAL,
    AST_INFIX_EXPRESSION,
    AST_FUNCTION,       // fn add(x, y) { ... }
    AST_FUNCTION_CALL   // add(5, 10)
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

// FIX: Added Function definition structure
typedef struct {
    Token token; // 'fn'
    Identifier* name;
    Identifier** parameters;
    size_t param_count;
    AstNode** body;
    size_t body_count;
} FunctionStatement;

// FIX: Added Function Call structure
typedef struct {
    Token token;
    Identifier* function_name;
    AstNode** arguments;
    size_t arg_count;
} FunctionCall;

struct AstNode {
    AstNodeType type;
    union {
        LetStatement let_stmt;
        ReturnStatement return_stmt;
        Identifier ident;
        IntegerLiteral int_literal;
        InfixExpression infix;
        AstNode* expression_stmt; 
        FunctionStatement function_stmt;
        FunctionCall function_call;
    } data;
};

typedef struct {
    AstNode** statements; 
    size_t statement_count;
    size_t statement_capacity;
} Program;

Program* create_program(Arena* arena);
void program_add_statement(Arena* arena, Program* program, AstNode* stmt);

#endif // AST_H
