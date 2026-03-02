#ifndef TOKEN_H
#define TOKEN_H

#include <stddef.h>

typedef enum {
    TOKEN_ILLEGAL,
    TOKEN_EOF,

    TOKEN_IDENT, 
    TOKEN_INT,   

    TOKEN_ASSIGN, // =
    TOKEN_PLUS,   // +
    TOKEN_MINUS,  // -
    TOKEN_ASTERISK, // *
    TOKEN_SLASH,    // /
    TOKEN_PERCENT,  // %

    TOKEN_COMMA,     // ,
    TOKEN_SEMICOLON, // ;
    TOKEN_LPAREN,    // (
    TOKEN_RPAREN,    // )
    TOKEN_LBRACE,    // {
    TOKEN_RBRACE,    // }

    TOKEN_LET,
    TOKEN_FN,
    TOKEN_RETURN
} TokenType;

typedef struct {
    TokenType type;
    const char* literal;
    size_t length;
    int line;
} Token;

#endif // TOKEN_H
