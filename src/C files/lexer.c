#include "lexer.h"
#include <string.h>
#include <stdbool.h>

// Helper to read the next character
static void read_char(Lexer* lexer) {
    if (lexer->read_position >= lexer->input_len) {
        lexer->ch = '\0';
    } else {
        lexer->ch = lexer->input[lexer->read_position];
    }
    lexer->position = lexer->read_position;
    lexer->read_position += 1;
}

// Helper to check if a character is a letter
static bool is_letter(char ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}

// Helper to check if a character is a digit
static bool is_digit(char ch) {
    return ch >= '0' && ch <= '9';
}

// Skip spaces, tabs, and newlines
static void skip_whitespace(Lexer* lexer) {
    while (lexer->ch == ' ' || lexer->ch == '\t' || lexer->ch == '\n' || lexer->ch == '\r') {
        if (lexer->ch == '\n') {
            lexer->line++;
        }
        read_char(lexer);
    }
}

// Create a token
static Token new_token(TokenType type, const char* literal, size_t length, int line) {
    Token tok;
    tok.type = type;
    tok.literal = literal;
    tok.length = length;
    tok.line = line;
    return tok;
}

// Check if an identifier is a keyword
static TokenType lookup_ident(const char* ident, size_t length) {
    if (length == 3 && strncmp(ident, "let", 3) == 0) return TOKEN_LET;
    if (length == 2 && strncmp(ident, "fn", 2) == 0) return TOKEN_FN;
    if (length == 6 && strncmp(ident, "return", 6) == 0) return TOKEN_RETURN;
    return TOKEN_IDENT;
}

void lexer_init(Lexer* lexer, const char* input, size_t input_len) {
    lexer->input = input;
    lexer->input_len = input_len;
    lexer->position = 0;
    lexer->read_position = 0;
    lexer->line = 1;
    read_char(lexer);
}

Token lexer_next_token(Lexer* lexer) {
    Token tok;

    skip_whitespace(lexer);

    switch (lexer->ch) {
        case '=': tok = new_token(TOKEN_ASSIGN, &lexer->input[lexer->position], 1, lexer->line); break;
        case ';': tok = new_token(TOKEN_SEMICOLON, &lexer->input[lexer->position], 1, lexer->line); break;
        case '(': tok = new_token(TOKEN_LPAREN, &lexer->input[lexer->position], 1, lexer->line); break;
        case ')': tok = new_token(TOKEN_RPAREN, &lexer->input[lexer->position], 1, lexer->line); break;
        case '{': tok = new_token(TOKEN_LBRACE, &lexer->input[lexer->position], 1, lexer->line); break;
        case '}': tok = new_token(TOKEN_RBRACE, &lexer->input[lexer->position], 1, lexer->line); break;
        case '+': tok = new_token(TOKEN_PLUS, &lexer->input[lexer->position], 1, lexer->line); break;
        case '-': tok = new_token(TOKEN_MINUS, &lexer->input[lexer->position], 1, lexer->line); break;
        case ',': tok = new_token(TOKEN_COMMA, &lexer->input[lexer->position], 1, lexer->line); break;
        case '\0':
            tok.type = TOKEN_EOF;
            tok.literal = "";
            tok.length = 0;
            tok.line = lexer->line;
            return tok;
        default:
            if (is_letter(lexer->ch)) {
                size_t start_pos = lexer->position;
                while (is_letter(lexer->ch)) {
                    read_char(lexer);
                }
                size_t length = lexer->position - start_pos;
                tok.type = lookup_ident(&lexer->input[start_pos], length);
                tok.literal = &lexer->input[start_pos];
                tok.length = length;
                tok.line = lexer->line;
                return tok; 
            } else if (is_digit(lexer->ch)) {
                size_t start_pos = lexer->position;
                while (is_digit(lexer->ch)) {
                    read_char(lexer);
                }
                size_t length = lexer->position - start_pos;
                tok = new_token(TOKEN_INT, &lexer->input[start_pos], length, lexer->line);
                return tok; 
            } else {
                tok = new_token(TOKEN_ILLEGAL, &lexer->input[lexer->position], 1, lexer->line);
            }
            break;
    }

    read_char(lexer);
    return tok;
}
