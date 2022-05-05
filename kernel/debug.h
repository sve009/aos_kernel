#pragma once

#include <stdint.h>

#define BREAK __asm__("int $3")

#define HEXDUMP 0
#define DECDUMP 1

/** 
 * Dump a region of memory starting at a given
 * point for debugging purposes.
 */
void dump_mem(uint64_t p, int flag, int rows, int cols);

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
