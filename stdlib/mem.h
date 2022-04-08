#pragma once

#include <stddef.h>
#include <stdint.h>

// Zero out memory region
void memset(void* start, uint64_t size);

// Copy memory region
void memcpy(void* dest, void* src, uint64_t size);

// Bump allocate memory
void* malloc(size_t sz);
