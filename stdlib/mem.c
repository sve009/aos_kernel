#include "mem.h"

#include <stdint.h>

#define PAGE_SIZE 2048

// Round a value x up to the next multiple of y
#define ROUND_UP(x, y) ((x) % (y) == 0 ? (x) : (x) + ((y) - (x) % (y)))

// Need to use mmap syscall
extern void syscall(int nr, ...);

// Useful for zeroing stuff out
void memset(void* start, uint64_t size) {
  uint64_t progress = 0;

  uint8_t z = 0;
  
  while (progress < size) {
    *((uint8_t*)((uint64_t)start + progress)) = z;
    progress += sizeof(z);
  }
}

// Useful for zeroing stuff out
void memcpy(void* dest, void* src, uint64_t size) {
  uint64_t progress = 0;

  uint8_t z = 0;
  
  while (progress < size) {
    *((uint8_t*)((uint64_t)dest + progress)) = *(uint8_t*)((uint64_t)src + progress);
    progress += sizeof(z);
  }
}

uint64_t mmap(void* address, size_t loc) {
  uint64_t p;
  syscall(2, &p);
  return p;
}

void* bump = NULL;
size_t space_remaining = 0;

void* malloc(size_t sz) {
  // Round sz up to a multiple of 16
  sz = ROUND_UP(sz, 16);

  // Do we have enough space to satisfy this allocation?
  if (space_remaining < sz) {
    // No. Get some more space using `mmap`
    size_t rounded_up = ROUND_UP(sz, PAGE_SIZE);
    void* newmem = mmap(NULL, rounded_up);

    // Check for errors
    if (newmem == NULL) {
      return NULL;
    }

    bump = newmem;
    space_remaining = rounded_up;
  }

  // Grab bytes from the beginning of our bump pointer region
  void* result = bump;
  bump += sz;
  space_remaining -= sz;

  return result;
}

void free(void* p) {
  // Do nothing
}


