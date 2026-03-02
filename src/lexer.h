#ifndef LEXER_H
#define LEXER_H

#include "token.h"

// Lexer state
typedef struct {
    const char* input;   // The complete source code loaded into memory
    size_t input_len;    // Total length of the source code
    size_t position;     // Current position in input (points to current char)
    size_t read_position;// Current reading position in input (after current char)
    char ch;             // Current char under examination
    int line;            // Current line number
} Lexer;

// Initialize the lexer with source code
void lexer_init(Lexer* lexer, const char* input, size_t input_len);

// Get the next token from the source
Token lexer_next_token(Lexer* lexer);

#endif // LEXER_H
