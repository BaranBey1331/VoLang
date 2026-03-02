#include "lexer.h"
#include <string.h>
#include <stdbool.h>

static void read_char(Lexer* lexer) {
    if (lexer->read_position >= lexer->input_len) {
        lexer->ch = '\0';
    } else {
        lexer->ch = lexer->input[lexer->read_position];
    }
    lexer->position = lexer->read_position;
    lexer->read_position += 1;
}

static bool is_letter(char ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}

static bool is_digit(char ch) {
    return ch >= '0' && ch <= '9';
}

// FIX: Highly optimized whitespace and invisible character skipper
static void skip_whitespace(Lexer* lexer) {
    while (1) {
        char c = lexer->ch;
        if (c == ' ' || c == '\t' || c == '\r') {
            read_char(lexer);
        } else if (c == '\n') {
            lexer->line++;
            read_char(lexer);
        } 
        // FIX: Ignore UTF-8 Zero Width Space (\xE2\x80\x8B) which causes Lexer errors
        else if (c == (char)0xE2) {
            if (lexer->read_position + 1 < lexer->input_len &&
                lexer->input[lexer->read_position] == (char)0x80 &&
                lexer->input[lexer->read_position + 1] == (char)0x8B) {
                read_char(lexer); // Skip E2
                read_char(lexer); // Skip 80
                read_char(lexer); // Skip 8B
            } else {
                break;
            }
        } else {
            break;
        }
    }
}

static Token new_token(TokenType type, const char* literal, size_t length, int line) {
    Token tok;
    tok.type = type;
    tok.literal = literal;
    tok.length = length;
    tok.line = line;
    return tok;
}

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
    
    // FIX: Skip UTF-8 BOM if present at the very beginning of the file
    if (input_len >= 3 && 
        (unsigned char)input[0] == 0xEF && 
        (unsigned char)input[1] == 0xBB && 
        (unsigned char)input[2] == 0xBF) {
        lexer->position = 3;
        lexer->read_position = 3;
    }
    
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
