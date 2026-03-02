#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stddef.h>

// Represents a tracked variable or function name
typedef struct {
    const char* name;
    size_t length;
    int index; // Where this variable is stored in memory/VM
} Symbol;

// The Symbol Table structure
typedef struct {
    Symbol* symbols;
    int count;
    int capacity;
} SymbolTable;

// Create a new Symbol Table
SymbolTable* symbol_table_create();

// Define a new variable in the table
Symbol* symbol_table_define(SymbolTable* st, const char* name, size_t length);

// Look up an existing variable
Symbol* symbol_table_resolve(SymbolTable* st, const char* name, size_t length);

// Free the table
void symbol_table_free(SymbolTable* st);

#endif // SYMBOL_TABLE_H
