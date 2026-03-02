#ifndef TOKEN_H
#define TOKEN_H

#include <stddef.h>

// Token types for Volang
typedef enum {
    TOKEN_ILLEGAL,
    TOKEN_EOF,

    // Identifiers and literals
    TOKEN_IDENT, // Variables, function names
    TOKEN_INT,   // 12345

    // Operators
    TOKEN_ASSIGN, // =
    TOKEN_PLUS,   // +
    TOKEN_MINUS,  // -

    // Delimiters
    TOKEN_COMMA,     // ,
    TOKEN_SEMICOLON, // ;
    TOKEN_LPAREN,    // (
    TOKEN_RPAREN,    // )
    TOKEN_LBRACE,    // {
    TOKEN_RBRACE,    // }

    // Keywords (English base, Turkish will be added later)
    TOKEN_LET,
    TOKEN_FN,
    TOKEN_RETURN
} TokenType;

// Zero-copy Token structure
typedef struct {
    TokenType type;
    const char* literal; // Pointer to the start of the token in the source file
    size_t length;       // Length of the token
    int line;            // Line number for error reporting
} Token;

#endif // TOKEN_H
