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
    if (argc < 2) {
        printf("--- %s Compiler v%s ---\n", VOLANG_NAME, VOLANG_VERSION);
        fprintf(stderr, "Usage: %s <file1%s> [file2%s] ...\n", argv[0], VOLANG_EXT, VOLANG_EXT);
        return 1;
    }

    // Initialize the global Memory Arena
    Arena arena;
    arena_init(&arena, 16 * 1024 * 1024); // 16MB for 1M+ LOC/s

    // Create the master program that will hold code from ALL files
    Program* master_program = create_program(&arena);

    // Keep track of source strings so we can free them at the very end
    char* loaded_sources[argc];
    memset(loaded_sources, 0, sizeof(loaded_sources));
    int parsed_file_count = 0;

    for (int i = 1; i < argc; i++) {
        const char* filename = argv[i];

        if (!check_extension(filename, VOLANG_EXT)) {
            fprintf(stderr, "Error: File '%s' must have a '%s' extension.\n", filename, VOLANG_EXT);
            continue; // Skip invalid files
        }

        FILE* file = fopen(filename, "rb");
        if (!file) {
            fprintf(stderr, "Error: Could not open file '%s'\n", filename);
            continue;
        }

        fseek(file, 0, SEEK_END);
        long fsize = ftell(file);
        fseek(file, 0, SEEK_SET);

        char* source_code = (char*)malloc(fsize + 1);
        size_t read_bytes = fread(source_code, 1, fsize, file);
        if (read_bytes != (size_t)fsize) {
            fprintf(stderr, "Warning: Failed to read entire file '%s'.\n", filename);
        }
        fclose(file);
        source_code[fsize] = '\0';
        
        loaded_sources[i] = source_code;
        parsed_file_count++;

        // Parse the current file
        Lexer lexer;
        lexer_init(&lexer, source_code, fsize);
        
        Parser parser;
        parser_init(&parser, &lexer, &arena);
        
        Program* file_program = parse_program(&parser);

        // Merge the file's AST into the master AST
        for (size_t stmt_idx = 0; stmt_idx < file_program->statement_count; stmt_idx++) {
            program_add_statement(&arena, master_program, file_program->statements[stmt_idx]);
        }
    }
    
    if (parsed_file_count == 0) {
        fprintf(stderr, "Error: No valid input files were parsed.\n");
        arena_free(&arena);
        return 1;
    }

    printf("[1/3] Multi-file AST generated successfully (%d file%s).\n",
           parsed_file_count,
           parsed_file_count == 1 ? "" : "s");

    // Optimize the combined AST
    Optimizer opt;
    optimizer_init(&opt);
    optimize_program(&opt, master_program);
    
    if (opt.folded_constants > 0 || opt.eliminated_nodes > 0) {
        printf("[2/3] AST Optimized: Folded %d constants, Eliminated %d dead nodes.\n", 
               opt.folded_constants, opt.eliminated_nodes);
    } else {
        printf("[2/3] AST Optimized: Clean.\n");
    }

    // Generate LLVM IR for the combined AST
    const char* output_ll = "output.ll";
    LLVMGen gen;
    llvm_gen_init(&gen, output_ll);
    
    if (llvm_gen_program(&gen, master_program)) {
        printf("[3/3] Native LLVM IR compiled to: %s\n", output_ll);
    } else {
        fprintf(stderr, "Compilation failed.\n");
        llvm_gen_close(&gen);
        arena_free(&arena);
        for (int i = 1; i < argc; i++) {
            if (loaded_sources[i]) free(loaded_sources[i]);
        }
        return 1;
    }

    // Cleanup
    llvm_gen_close(&gen);
    arena_free(&arena);
    
    for (int i = 1; i < argc; i++) {
        if (loaded_sources[i]) free(loaded_sources[i]);
    }
    
    return 0;
}
