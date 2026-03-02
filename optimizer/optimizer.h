#ifndef OPTIMIZER_H
#define OPTIMIZER_H

// Since we added -I"src/H_files" in the Makefile, 
// we don't need relative paths anymore. GCC will find it automatically.
#include "ast.h"
#include <stdbool.h>

// Optimizer state and statistics
typedef struct {
    int folded_constants;
    int eliminated_nodes;
} Optimizer;

// Initialize the optimizer
void optimizer_init(Optimizer* opt);

// Optimize the entire AST Program in-place
void optimize_program(Optimizer* opt, Program* program);

#endif // OPTIMIZER_H
