#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

static void next_token(Parser* parser) {
    parser->current_token = parser->peek_token;
    parser->peek_token = lexer_next_token(parser->lexer);
}

static int current_token_is(Parser* parser, TokenType type) {
    return parser->current_token.type == type;
}

static int peek_token_is(Parser* parser, TokenType type) {
    return parser->peek_token.type == type;
}

static int expect_peek(Parser* parser, TokenType type) {
    if (peek_token_is(parser, type)) {
        next_token(parser);
        return 1;
    } else {
        fprintf(stderr, "Syntax Error on line %d: Expected token %d, got %d\n", 
                parser->peek_token.line, type, parser->peek_token.type);
        return 0;
    }
}

void parser_init(Parser* parser, Lexer* lexer, Arena* arena) {
    parser->lexer = lexer;
    parser->arena = arena;
    next_token(parser);
    next_token(parser);
}

static AstNode* parse_expression(Parser* parser, int precedence);

static AstNode* parse_identifier(Parser* parser) {
    AstNode* node = (AstNode*)arena_alloc(parser->arena, sizeof(AstNode));
    node->type = AST_IDENTIFIER;
    node->data.ident.token = parser->current_token;
    node->data.ident.value = parser->current_token.literal;
    node->data.ident.length = parser->current_token.length;
    return node;
}

static AstNode* parse_integer_literal(Parser* parser) {
    AstNode* node = (AstNode*)arena_alloc(parser->arena, sizeof(AstNode));
    node->type = AST_INTEGER_LITERAL;
    node->data.int_literal.token = parser->current_token;
    
    char buffer[32] = {0};
    size_t len = parser->current_token.length < 31 ? parser->current_token.length : 31;
    for(size_t i=0; i<len; i++) buffer[i] = parser->current_token.literal[i];
    node->data.int_literal.value = atoll(buffer);
    
    return node;
}

static AstNode* parse_expression(Parser* parser, int precedence) {
    (void)precedence; 
    AstNode* left_exp = NULL;

    if (current_token_is(parser, TOKEN_IDENT)) {
        left_exp = parse_identifier(parser);
    } else if (current_token_is(parser, TOKEN_INT)) {
        left_exp = parse_integer_literal(parser);
    }

    if (peek_token_is(parser, TOKEN_PLUS) || peek_token_is(parser, TOKEN_MINUS)) {
        next_token(parser);
        AstNode* infix_node = (AstNode*)arena_alloc(parser->arena, sizeof(AstNode));
        infix_node->type = AST_INFIX_EXPRESSION;
        infix_node->data.infix.token = parser->current_token;
        infix_node->data.infix.operator_str = parser->current_token.literal;
        infix_node->data.infix.operator_len = parser->current_token.length;
        infix_node->data.infix.left = left_exp;
        
        next_token(parser);
        infix_node->data.infix.right = parse_expression(parser, 0);
        return infix_node;
    }

    return left_exp;
}

static AstNode* parse_let_statement(Parser* parser) {
    AstNode* node = (AstNode*)arena_alloc(parser->arena, sizeof(AstNode));
    node->type = AST_LET_STATEMENT;
    node->data.let_stmt.token = parser->current_token;

    if (!expect_peek(parser, TOKEN_IDENT)) return NULL;

    node->data.let_stmt.name = (Identifier*)arena_alloc(parser->arena, sizeof(Identifier));
    node->data.let_stmt.name->token = parser->current_token;
    node->data.let_stmt.name->value = parser->current_token.literal;
    node->data.let_stmt.name->length = parser->current_token.length;

    if (!expect_peek(parser, TOKEN_ASSIGN)) return NULL;

    next_token(parser); 
    node->data.let_stmt.value = parse_expression(parser, 0);

    if (peek_token_is(parser, TOKEN_SEMICOLON)) {
        next_token(parser);
    }

    return node;
}

static AstNode* parse_return_statement(Parser* parser) {
    AstNode* node = (AstNode*)arena_alloc(parser->arena, sizeof(AstNode));
    node->type = AST_RETURN_STATEMENT;
    node->data.return_stmt.token = parser->current_token;

    next_token(parser);
    node->data.return_stmt.return_value = parse_expression(parser, 0);

    if (peek_token_is(parser, TOKEN_SEMICOLON)) {
        next_token(parser);
    }

    return node;
}

static AstNode* parse_statement(Parser* parser) {
    switch (parser->current_token.type) {
        case TOKEN_LET:
            return parse_let_statement(parser);
        case TOKEN_RETURN:
            return parse_return_statement(parser);
        default:
            return NULL; 
    }
}

Program* parse_program(Parser* parser) {
    Program* program = create_program(parser->arena);

    while (parser->current_token.type != TOKEN_EOF) {
        AstNode* stmt = parse_statement(parser);
        if (stmt != NULL) {
            program_add_statement(parser->arena, program, stmt);
        }
        next_token(parser);
    }

    return program;
}
