#include "debug.h"

#include "kprint.h"

#define ROWSIZE 6

/**
 * Some quick math:
 *   Desired: 10 rows with 32 bytes each.
 *
 *   So: Display as 8 ints in each row.
 *       Thus we will display the next 80 ints
 */
void dump_mem(uint64_t start, int flag, int rows, int cols) {
  // Recast so math is easier
  uint16_t* p = (uint16_t*)start;

  // Iterate through rows
  for (int i = 0; i < rows; i++) {
    // Print address header
    kprintf("%p:", p + (cols*i));

    // Iterate through vals
    for (int j = 0; j < cols; j++) {
      int val = *(p + (cols*i) + j);
      if (flag == HEXDUMP) {
        kprintf(" %x", val);
      } else {
        kprintf(" %d", val);
      }
    }

    // Break line
    kprintf("\n");
  }

}

// Shell function
// Notice that this is just an
//  estimate, as the sp isn't exact
void print_int(uint64_t p) {
  dump_mem(p, HEXDUMP, 1, 2);
}

// Use a quick hack to get stack pointer
void dump_stack() {
  // Add something to stack
  int dummy;

  // Get address
  uint64_t p = &dummy;

  // Dump
  dump_mem(p, HEXDUMP, 16, 10);
}

