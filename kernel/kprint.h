#pragma once

#include <stddef.h>

// Set the terminal_writing function
typedef void (*term_write_t)(const char*, size_t);

void set_term_write(term_write_t fn);

void kprintf(const char* format, ...);
