#include "symbol_table.h"
#include <stdlib.h>
#include <string.h>

SymbolTable* symbol_table_create() {
    SymbolTable* st = (SymbolTable*)malloc(sizeof(SymbolTable));
    st->capacity = 16; // Initial capacity
    st->count = 0;
    st->symbols = (Symbol*)malloc(sizeof(Symbol) * st->capacity);
    return st;
}

Symbol* symbol_table_define(SymbolTable* st, const char* name, size_t length) {
    if (st->count >= st->capacity) {
        st->capacity *= 2;
        st->symbols = (Symbol*)realloc(st->symbols, sizeof(Symbol) * st->capacity);
    }

    Symbol* symbol = &st->symbols[st->count];
    symbol->name = name;
    symbol->length = length;
    symbol->index = st->count; // Assign an index for VM to track it
    
    st->count++;
    return symbol;
}

Symbol* symbol_table_resolve(SymbolTable* st, const char* name, size_t length) {
    // Reverse loop to find the most recently defined variable first (helps with scoping)
    for (int i = st->count - 1; i >= 0; i--) {
        Symbol* sym = &st->symbols[i];
        if (sym->length == length && strncmp(sym->name, name, length) == 0) {
            return sym;
        }
    }
    return NULL; // Not found
}

void symbol_table_free(SymbolTable* st) {
    free(st->symbols);
    free(st);
}
