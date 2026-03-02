#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file.vo>\n", argv[0]);
        return 1;
    }

    const char* filename = argv[1];
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return 1;
    }

    // Read file into memory (Zero-copy architecture)
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* source_code = malloc(fsize + 1);
    fread(source_code, fsize, 1, file);
    fclose(file);
    source_code[fsize] = '\0';

    // 1. Initialize Lexer
    Lexer lexer;
    lexer_init(&lexer, source_code, fsize);

    // 2. Initialize Memory Arena (Allocate 1MB block for the AST instantly)
    Arena arena;
    arena_init(&arena, 1024 * 1024);

    // 3. Initialize Parser
    Parser parser;
    parser_init(&parser, &lexer, &arena);

    // 4. Parse the program into an Abstract Syntax Tree
    printf("Compiling %s...\n", filename);
    Program* program = parse_program(&parser);

    // 5. Output the structured AST to verify correctness
    printf("--- Volang Abstract Syntax Tree (AST) ---\n");
    for (size_t i = 0; i < program->statement_count; i++) {
        print_ast(program->statements[i], 0);
    }

    // Cleanup
    arena_free(&arena);
    free(source_code);
    return 0;
}
