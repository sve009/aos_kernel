#pragma once

#include <stdint.h>

#include "stivale2.h"

#define BREAK __asm__("int $3")

#define HEXDUMP 0
#define DECDUMP 1

/**
 * Find the symbol table and string tables
 */
void init_tables(struct stivale2_struct_tag_modules* tag);

/**
 * Takes a null-terminated string,
 * and returns the value (most likely address)
 * associated with it in the kernel binary.
 *
 * Returns null if symbol not found
 */
uint64_t lookup_symbol(char* symbol);

/** 
 * Dump a region of memory starting at a given
 * point for debugging purposes.
 */
void dump_mem(uint64_t p, int flag, int rows, int cols);

uintptr_t get_addr();

/**
 * Prints out single integer value
 */
void print_int(uint64_t p);

/**
 * Dumps out contents of top of stack
 *
 * CAUTION: Simply an estimate
 */
void dump_stack();
