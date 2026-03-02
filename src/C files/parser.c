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
static AstNode* parse_statement(Parser* parser);

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

// FIX: Added function call parsing (e.g., add(five, ten))
static AstNode* parse_function_call(Parser* parser, AstNode* function_ident) {
    AstNode* node = (AstNode*)arena_alloc(parser->arena, sizeof(AstNode));
    node->type = AST_FUNCTION_CALL;
    node->data.function_call.token = parser->current_token; // '('
    node->data.function_call.function_name = &function_ident->data.ident;
    
    // Allocate space for up to 8 arguments for extreme speed (no realloc)
    node->data.function_call.arguments = (AstNode**)arena_alloc(parser->arena, sizeof(AstNode*) * 8);
    node->data.function_call.arg_count = 0;
    
    next_token(parser); // Move past '('
    
    if (!current_token_is(parser, TOKEN_RPAREN)) {
        node->data.function_call.arguments[node->data.function_call.arg_count++] = parse_expression(parser, 0);
        
        while (peek_token_is(parser, TOKEN_COMMA)) {
            next_token(parser); // move to comma
            next_token(parser); // move past comma
            node->data.function_call.arguments[node->data.function_call.arg_count++] = parse_expression(parser, 0);
        }
    }
    
    if (!expect_peek(parser, TOKEN_RPAREN)) return NULL;
    
    return node;
}

static AstNode* parse_expression(Parser* parser, int precedence) {
    (void)precedence; 
    AstNode* left_exp = NULL;

    if (current_token_is(parser, TOKEN_IDENT)) {
        left_exp = parse_identifier(parser);
        
        // If an identifier is followed by '(', it's a function call!
        if (peek_token_is(parser, TOKEN_LPAREN)) {
            left_exp = parse_function_call(parser, left_exp);
        }
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

// FIX: Added 'fn' (Function) Parsing implementation
static AstNode* parse_function_statement(Parser* parser) {
    AstNode* node = (AstNode*)arena_alloc(parser->arena, sizeof(AstNode));
    node->type = AST_FUNCTION;
    node->data.function_stmt.token = parser->current_token; // 'fn'

    if (!expect_peek(parser, TOKEN_IDENT)) return NULL;

    node->data.function_stmt.name = (Identifier*)arena_alloc(parser->arena, sizeof(Identifier));
    node->data.function_stmt.name->token = parser->current_token;
    node->data.function_stmt.name->value = parser->current_token.literal;
    node->data.function_stmt.name->length = parser->current_token.length;

    if (!expect_peek(parser, TOKEN_LPAREN)) return NULL;

    // Parse Parameters (Fast array allocation for up to 8 params)
    node->data.function_stmt.parameters = (Identifier**)arena_alloc(parser->arena, sizeof(Identifier*) * 8);
    node->data.function_stmt.param_count = 0;

    next_token(parser); // Move past '('

    if (!current_token_is(parser, TOKEN_RPAREN)) {
        Identifier* param = (Identifier*)arena_alloc(parser->arena, sizeof(Identifier));
        param->value = parser->current_token.literal;
        param->length = parser->current_token.length;
        node->data.function_stmt.parameters[node->data.function_stmt.param_count++] = param;

        while (peek_token_is(parser, TOKEN_COMMA)) {
            next_token(parser); // move to comma
            next_token(parser); // move past comma
            
            Identifier* next_param = (Identifier*)arena_alloc(parser->arena, sizeof(Identifier));
            next_param->value = parser->current_token.literal;
            next_param->length = parser->current_token.length;
            node->data.function_stmt.parameters[node->data.function_stmt.param_count++] = next_param;
        }
    }

    if (!expect_peek(parser, TOKEN_RPAREN)) return NULL;
    if (!expect_peek(parser, TOKEN_LBRACE)) return NULL;

    // Parse Block Body
    node->data.function_stmt.body = (AstNode**)arena_alloc(parser->arena, sizeof(AstNode*) * 64);
    node->data.function_stmt.body_count = 0;

    next_token(parser); // Move past '{'

    while (!current_token_is(parser, TOKEN_RBRACE) && !current_token_is(parser, TOKEN_EOF)) {
        AstNode* stmt = parse_statement(parser);
        if (stmt != NULL) {
            node->data.function_stmt.body[node->data.function_stmt.body_count++] = stmt;
        }
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
        case TOKEN_FN:
            return parse_function_statement(parser);
        default:
            // For now, treat standalone expressions (like variable declarations without let) as null or parse them
            // We return NULL to skip unrecognized to avoid infinite loops
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
