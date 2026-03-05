#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        fprintf(stderr, "Syntax Error on line %d: Expected token %d, got %d ('%.*s')\n", 
                parser->peek_token.line, type, parser->peek_token.type,
                (int)parser->peek_token.length, parser->peek_token.literal);
        return 0;
    }
}

static int token_precedence(TokenType type) {
    switch (type) {
        case TOKEN_PLUS:
        case TOKEN_MINUS:
            return 10;
        case TOKEN_ASTERISK:
        case TOKEN_SLASH:
        case TOKEN_PERCENT:
            return 20;
        default:
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
    node->data_type = VO_TYPE_UNKNOWN;
    node->data.ident.token = parser->current_token;
    node->data.ident.value = parser->current_token.literal;
    node->data.ident.length = parser->current_token.length;
    return node;
}

static AstNode* parse_integer_literal(Parser* parser) {
    AstNode* node = (AstNode*)arena_alloc(parser->arena, sizeof(AstNode));
    node->type = AST_INTEGER_LITERAL;
    node->data_type = VO_TYPE_I64;
    node->data.int_literal.token = parser->current_token;
    
    char buffer[32] = {0};
    size_t len = parser->current_token.length < 31 ? parser->current_token.length : 31;
    for(size_t i=0; i<len; i++) buffer[i] = parser->current_token.literal[i];
    node->data.int_literal.value = atoll(buffer);
    
    return node;
}

static AstNode* parse_function_call(Parser* parser, AstNode* function_ident) {
    AstNode* node = (AstNode*)arena_alloc(parser->arena, sizeof(AstNode));
    node->type = AST_FUNCTION_CALL;
    node->data_type = VO_TYPE_UNKNOWN;
    node->data.function_call.token = parser->current_token; // '('
    node->data.function_call.function_name = &function_ident->data.ident;
    
    // Dynamic array logic to remove fixed limits safely
    size_t temp_cap = 8;
    size_t temp_count = 0;
    AstNode** temp_args = (AstNode**)malloc(temp_cap * sizeof(AstNode*));
    
    next_token(parser); // Move past '('
    
    if (!current_token_is(parser, TOKEN_RPAREN)) {
        temp_args[temp_count++] = parse_expression(parser, 0);
        
        while (peek_token_is(parser, TOKEN_COMMA)) {
            next_token(parser); 
            next_token(parser); 
            
            if (temp_count >= temp_cap) {
                temp_cap *= 2;
                temp_args = (AstNode**)realloc(temp_args, temp_cap * sizeof(AstNode*));
            }
            temp_args[temp_count++] = parse_expression(parser, 0);
        }
    }
    
    if (!expect_peek(parser, TOKEN_RPAREN)) {
        free(temp_args);
        return NULL;
    }
    
    // Move dynamically sized array perfectly into the Arena
    if (temp_count > 0) {
        node->data.function_call.arguments = (AstNode**)arena_alloc(parser->arena, temp_count * sizeof(AstNode*));
        memcpy(node->data.function_call.arguments, temp_args, temp_count * sizeof(AstNode*));
    } else {
        node->data.function_call.arguments = NULL;
    }
    node->data.function_call.arg_count = temp_count;
    
    free(temp_args);
    return node;
}

static AstNode* parse_expression(Parser* parser, int precedence) {
    AstNode* left_exp = NULL;

    if (current_token_is(parser, TOKEN_IDENT)) {
        left_exp = parse_identifier(parser);
        
        if (peek_token_is(parser, TOKEN_LPAREN)) {
            next_token(parser); 
            left_exp = parse_function_call(parser, left_exp);
        }
    } else if (current_token_is(parser, TOKEN_INT)) {
        left_exp = parse_integer_literal(parser);
    }

    if (!left_exp) {
        return NULL;
    }

    while (token_precedence(parser->peek_token.type) > precedence) {
        next_token(parser);
        AstNode* infix_node = (AstNode*)arena_alloc(parser->arena, sizeof(AstNode));
        infix_node->type = AST_INFIX_EXPRESSION;
        infix_node->data_type = VO_TYPE_UNKNOWN;
        infix_node->data.infix.token = parser->current_token;
        infix_node->data.infix.operator_str = parser->current_token.literal;
        infix_node->data.infix.operator_len = parser->current_token.length;
        infix_node->data.infix.left = left_exp;

        int current_precedence = token_precedence(parser->current_token.type);
        next_token(parser);
        infix_node->data.infix.right = parse_expression(parser, current_precedence);
        left_exp = infix_node;
    }

    return left_exp;
}

static AstNode* parse_let_statement(Parser* parser) {
    AstNode* node = (AstNode*)arena_alloc(parser->arena, sizeof(AstNode));
    node->type = AST_LET_STATEMENT;
    node->data_type = VO_TYPE_UNKNOWN;
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
    node->data_type = VO_TYPE_UNKNOWN;
    node->data.return_stmt.token = parser->current_token;

    next_token(parser);
    node->data.return_stmt.return_value = parse_expression(parser, 0);

    if (peek_token_is(parser, TOKEN_SEMICOLON)) {
        next_token(parser);
    }

    return node;
}

static AstNode* parse_function_statement(Parser* parser) {
    AstNode* node = (AstNode*)arena_alloc(parser->arena, sizeof(AstNode));
    node->type = AST_FUNCTION;
    node->data_type = VO_TYPE_UNKNOWN;
    node->data.function_stmt.token = parser->current_token; 

    if (!expect_peek(parser, TOKEN_IDENT)) return NULL;

    node->data.function_stmt.name = (Identifier*)arena_alloc(parser->arena, sizeof(Identifier));
    node->data.function_stmt.name->token = parser->current_token;
    node->data.function_stmt.name->value = parser->current_token.literal;
    node->data.function_stmt.name->length = parser->current_token.length;

    if (!expect_peek(parser, TOKEN_LPAREN)) return NULL;

    // Dynamic Parameter Parsing
    size_t temp_param_cap = 8;
    size_t temp_param_count = 0;
    Identifier** temp_params = (Identifier**)malloc(temp_param_cap * sizeof(Identifier*));

    next_token(parser); 

    if (!current_token_is(parser, TOKEN_RPAREN)) {
        Identifier* param = (Identifier*)arena_alloc(parser->arena, sizeof(Identifier));
        param->value = parser->current_token.literal;
        param->length = parser->current_token.length;
        temp_params[temp_param_count++] = param;

        while (peek_token_is(parser, TOKEN_COMMA)) {
            next_token(parser); 
            next_token(parser); 
            
            if (temp_param_count >= temp_param_cap) {
                temp_param_cap *= 2;
                temp_params = (Identifier**)realloc(temp_params, temp_param_cap * sizeof(Identifier*));
            }
            
            Identifier* next_param = (Identifier*)arena_alloc(parser->arena, sizeof(Identifier));
            next_param->value = parser->current_token.literal;
            next_param->length = parser->current_token.length;
            temp_params[temp_param_count++] = next_param;
        }
    }

    if (!expect_peek(parser, TOKEN_RPAREN)) { free(temp_params); return NULL; }
    if (!expect_peek(parser, TOKEN_LBRACE)) { free(temp_params); return NULL; }

    if (temp_param_count > 0) {
        node->data.function_stmt.parameters = (Identifier**)arena_alloc(parser->arena, temp_param_count * sizeof(Identifier*));
        memcpy(node->data.function_stmt.parameters, temp_params, temp_param_count * sizeof(Identifier*));
    } else {
        node->data.function_stmt.parameters = NULL;
    }
    node->data.function_stmt.param_count = temp_param_count;
    free(temp_params);

    // Dynamic Body Parsing
    size_t temp_body_cap = 16;
    size_t temp_body_count = 0;
    AstNode** temp_body = (AstNode**)malloc(temp_body_cap * sizeof(AstNode*));

    next_token(parser); 

    while (!current_token_is(parser, TOKEN_RBRACE) && !current_token_is(parser, TOKEN_EOF)) {
        AstNode* stmt = parse_statement(parser);
        if (stmt != NULL) {
            if (temp_body_count >= temp_body_cap) {
                temp_body_cap *= 2;
                temp_body = (AstNode**)realloc(temp_body, temp_body_cap * sizeof(AstNode*));
            }
            temp_body[temp_body_count++] = stmt;
        }
        next_token(parser);
    }

    if (temp_body_count > 0) {
        node->data.function_stmt.body = (AstNode**)arena_alloc(parser->arena, temp_body_count * sizeof(AstNode*));
        memcpy(node->data.function_stmt.body, temp_body, temp_body_count * sizeof(AstNode*));
    } else {
        node->data.function_stmt.body = NULL;
    }
    node->data.function_stmt.body_count = temp_body_count;
    free(temp_body);

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
