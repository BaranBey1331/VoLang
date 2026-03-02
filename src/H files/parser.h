#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

// The Parser state
typedef struct {
    Lexer* lexer;
    Arena* arena; 
    
    Token current_token;
    Token peek_token;
} Parser;

void parser_init(Parser* parser, Lexer* lexer, Arena* arena);
Program* parse_program(Parser* parser);

#endif // PARSER_H
