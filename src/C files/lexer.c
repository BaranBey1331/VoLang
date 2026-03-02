#include "lexer.h"
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

static void skip_whitespace(Lexer* lexer) {
    while (1) {
        char c = lexer->ch;
        if (c == ' ' || c == '\t' || c == '\r') {
            read_char(lexer);
        } else if (c == '\n') {
            lexer->line++;
            read_char(lexer);
        } 
        else if (c == (char)0xE2) {
            if (lexer->read_position + 1 < lexer->input_len &&
                lexer->input[lexer->read_position] == (char)0x80 &&
                lexer->input[lexer->read_position + 1] == (char)0x8B) {
                read_char(lexer); 
                read_char(lexer); 
                read_char(lexer); 
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

// 1M LOC/s OPTIMIZATION: Replaced slow 'strncmp' with direct length-indexed character matching
static TokenType lookup_ident(const char* ident, size_t length) {
    if (length == 2 && ident[0] == 'f' && ident[1] == 'n') {
        return TOKEN_FN;
    }
    if (length == 3 && ident[0] == 'l' && ident[1] == 'e' && ident[2] == 't') {
        return TOKEN_LET;
    }
    if (length == 6 && ident[0] == 'r' && ident[1] == 'e' && ident[2] == 't' && 
        ident[3] == 'u' && ident[4] == 'r' && ident[5] == 'n') {
        return TOKEN_RETURN;
    }
    return TOKEN_IDENT;
}

void lexer_init(Lexer* lexer, const char* input, size_t input_len) {
    lexer->input = input;
    lexer->input_len = input_len;
    lexer->position = 0;
    lexer->read_position = 0;
    lexer->line = 1;
    
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
                while (is_letter(lexer->ch) || is_digit(lexer->ch)) { // Added digit support for names like 'var1'
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
