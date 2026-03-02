#include "optimizer.h"
#include <stdio.h>

void optimizer_init(Optimizer* opt) {
    opt->folded_constants = 0;
    opt->eliminated_nodes = 0;
}

// Recursive function to optimize a single AST node IN-PLACE
// In-place mutation is a custom VoLang method to achieve crazy Loc/s without re-allocating memory.
static void optimize_node(Optimizer* opt, AstNode* node) {
    if (!node) return;

    switch (node->type) {
        case AST_LET_STATEMENT:
            optimize_node(opt, node->data.let_stmt.value);
            break;

        case AST_RETURN_STATEMENT:
            optimize_node(opt, node->data.return_stmt.return_value);
            break;

        case AST_EXPRESSION_STATEMENT:
            optimize_node(opt, node->data.expression_stmt);
            break;

        case AST_INFIX_EXPRESSION:
            // First, optimize the children (post-order traversal)
            optimize_node(opt, node->data.infix.left);
            optimize_node(opt, node->data.infix.right);

            // CONSTANT FOLDING ALGORITHM
            // If both left and right sides are pure integers, we calculate the result NOW.
            if (node->data.infix.left->type == AST_INTEGER_LITERAL && 
                node->data.infix.right->type == AST_INTEGER_LITERAL) {
                
                long long left_val = node->data.infix.left->data.int_literal.value;
                long long right_val = node->data.infix.right->data.int_literal.value;
                long long result = 0;
                
                if (node->data.infix.operator_str[0] == '+') {
                    result = left_val + right_val;
                } else if (node->data.infix.operator_str[0] == '-') {
                    result = left_val - right_val;
                }
                
                // IN-PLACE MUTATION: 
                // We change this AST_INFIX_EXPRESSION node directly into an AST_INTEGER_LITERAL.
                // This costs ZERO memory allocations. Faster than C++!
                node->type = AST_INTEGER_LITERAL;
                node->data.int_literal.value = result;
                
                opt->folded_constants++;
            }
            break;

        default:
            break;
    }
}

void optimize_program(Optimizer* opt, Program* program) {
    int valid_statements = 0;
    bool found_return = false;

    // Iterate through all statements in the program
    for (size_t i = 0; i < program->statement_count; i++) {
        
        // DEAD CODE ELIMINATION
        // If we already hit a return statement in this block, anything after is dead code.
        if (found_return) {
            opt->eliminated_nodes++;
            continue; // Skip processing and generating LLVM IR for this node
        }

        optimize_node(opt, program->statements[i]);
        
        // Keep the valid statement
        program->statements[valid_statements++] = program->statements[i];

        if (program->statements[i]->type == AST_RETURN_STATEMENT) {
            found_return = true;
        }
    }

    // Shrink the program if we eliminated dead code
    program->statement_count = valid_statements;
}
