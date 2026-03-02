#ifndef OPCODE_H
#define OPCODE_H

#include <stdint.h>

// VoLang Bytecode Instructions (Opcodes)
typedef enum {
    OP_CONSTANT,    // Load a constant (like a number)
    OP_ADD,         // Add two numbers
    OP_SUBTRACT,    // Subtract two numbers
    OP_SET_GLOBAL,  // Save a variable (let x = ...)
    OP_GET_GLOBAL,  // Retrieve a variable
    OP_RETURN       // Return from function
} OpCode;

#endif // OPCODE_H
