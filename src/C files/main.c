#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "lexer.h"
#include "parser.h"
#include "codegen.h"

int check_extension(const char* filename, const char* ext) {
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename) return 0;
    return strcmp(dot, ext) == 0;
}

int main(int argc, char* argv[]) {
    printf("--- %s Native Compiler v%s ---\n", VOLANG_NAME, VOLANG_VERSION);

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file%s>\n", argv[0], VOLANG_EXT);
        return 1;
    }

    const char* filename = argv[1];

    if (!check_extension(filename, VOLANG_EXT)) {
        fprintf(stderr, "Error: File must have a '%s' extension.\n", VOLANG_EXT);
        return 1;
    }

    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return 1;
    }

    // Fast zero-copy file read
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* source_code = (char*)malloc(fsize + 1);
    fread(source_code, fsize, 1, file);
    fclose(file);
    source_code[fsize] = '\0';

    // 1. Lexical Analysis
    Lexer lexer;
    lexer_init(&lexer, source_code, fsize);

    // 2. Parser & Memory Arena
    Arena arena;
    arena_init(&arena, 1024 * 1024); // 1MB AST Memory
    Parser parser;
    parser_init(&parser, &lexer, &arena);
    Program* program = parse_program(&parser);
    printf("[1/2] AST generated successfully.\n");

    // 3. Native C Code Generation (Backend)
    const char* output_filename = "output.c";
    CodeGen cg;
    codegen_init(&cg, output_filename);
    
    if (codegen_program(&cg, program)) {
        printf("[2/2] Native C code generated: %s\n", output_filename);
        printf("\nTo compile to machine code, run:\n");
        printf("gcc -O3 %s -o app\n", output_filename);
    } else {
        fprintf(stderr, "Code generation failed.\n");
    }

    codegen_close(&cg);

    // Cleanup
    arena_free(&arena);
    free(source_code);
    return 0;
}
