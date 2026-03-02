#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "lexer.h"
#include "parser.h"
#include "optimizer.h"
#include "llvm_gen.h"
#include "plugin.h"

int check_extension(const char* filename, const char* ext) {
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename) return 0;
    return strcmp(dot, ext) == 0;
}

int main(int argc, char* argv[]) {
    printf("--- %s Fast Native Compiler v%s ---\n", VOLANG_NAME, VOLANG_VERSION);

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

    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* source_code = (char*)malloc(fsize + 1);
    fread(source_code, fsize, 1, file);
    fclose(file);
    source_code[fsize] = '\0';

    // 1. Lexical Analysis (Zero-Copy)
    Lexer lexer;
    lexer_init(&lexer, source_code, fsize);

    // 2. Parser & Arena (O(1) Allocations)
    Arena arena;
    arena_init(&arena, 1024 * 1024); // 1MB AST Memory
    Parser parser;
    parser_init(&parser, &lexer, &arena);
    Program* program = parse_program(&parser);
    
    printf("[1/3] AST generated in memory.\n");

    // 3. Custom AST Optimizer (In-place Mutation)
    Optimizer opt;
    optimizer_init(&opt);
    optimize_program(&opt, program);
    
    if (opt.folded_constants > 0 || opt.eliminated_nodes > 0) {
        printf("[2/3] AST Optimized: Folded %d constants, Eliminated %d dead nodes.\n", 
               opt.folded_constants, opt.eliminated_nodes);
    } else {
        printf("[2/3] AST Optimized: No optimizations needed.\n");
    }

    // 4. Generate Native LLVM IR
    const char* output_ll = "output.ll";
    LLVMGen gen;
    llvm_gen_init(&gen, output_ll);
    
    if (llvm_gen_program(&gen, program)) {
        printf("[3/3] Native LLVM IR compiled to: %s\n", output_ll);
    } else {
        fprintf(stderr, "Compilation failed.\n");
    }

    // Cleanup
    llvm_gen_close(&gen);
    arena_free(&arena);
    free(source_code);
    return 0;
}
