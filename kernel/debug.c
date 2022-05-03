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
void dump_mem(uint64_t start, int flag) {
  // Recast so math is easier
  uint16_t* p = (uint16_t*)start;

  // Iterate through rows
  for (int i = 0; i < 10; i++) {
    // Print address header
    kprintf("%p:", p + (ROWSIZE*i));

    // Iterate through vals
    for (int j = 0; j < ROWSIZE; j++) {
      int val = *(p + (ROWSIZE*i) + j);
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
