#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "lexer.h"
#include "parser.h"
#include "compiler.h"

// Helper function to check if the file has the correct VoLang extension
int check_extension(const char* filename, const char* ext) {
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename) return 0;
    return strcmp(dot, ext) == 0;
}

int main(int argc, char* argv[]) {
    printf("--- %s Compiler v%s ---\n", VOLANG_NAME, VOLANG_VERSION);

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

    // Zero-copy architecture read
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* source_code = (char*)malloc(fsize + 1);
    fread(source_code, fsize, 1, file);
    fclose(file);
    source_code[fsize] = '\0';

    // 1. Lexer
    Lexer lexer;
    lexer_init(&lexer, source_code, fsize);

    // 2. Parser & Arena
    Arena arena;
    arena_init(&arena, 1024 * 1024); // 1MB AST Memory
    Parser parser;
    parser_init(&parser, &lexer, &arena);
    Program* program = parse_program(&parser);

    printf("[1/2] AST generated successfully.\n");

    // 3. Compiler (AST -> Bytecode)
    Compiler* compiler = compiler_create();
    if (compile_program(compiler, program)) {
        printf("[2/2] Bytecode compiled successfully! (%d bytes)\n", compiler->bytecode.length);
        
        // Print generated opcodes for debugging
        printf("\nGenerated Opcodes:\n");
        for (int i = 0; i < compiler->bytecode.length; i++) {
            printf("%04d | OP_CODE: %d\n", i, compiler->bytecode.instructions[i]);
        }
    } else {
        fprintf(stderr, "Compilation failed.\n");
    }

    // Cleanup
    compiler_free(compiler);
    arena_free(&arena);
    free(source_code);
    return 0;
}
