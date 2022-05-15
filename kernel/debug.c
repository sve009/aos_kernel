#include "debug.h"

#include "kprint.h"

#define ROWSIZE 6

// Reads address from user
uintptr_t get_addr() {
	uintptr_t addr = 0x0;
	uint64_t nums[16];
	uint64_t curr_ind = 0;

	kprintf("Input address -> 0x");
	char curr_char = 0;
	while ((curr_char = kgetc()) != '\n') {

		//if we have made it to the end then we set the ind to the last num in array and break
		if (curr_ind == 16) {
		 curr_ind = 15;
		 break;
		}	 
		
		kprintf("%c", curr_char);
		//translating the characters into numerical hex values
		if (curr_char <= '0' && curr_char >= '9') {
			nums[curr_ind] = (uint64_t) curr_char - '0';
		} else if (curr_char <= 'A' && curr_char >= 'F') {
			nums[curr_ind] = (uint64_t) curr_char - 'A' + 10;
		} else if (curr_char <= 'a' && curr_char >= 'f') {
			nums[curr_ind] = (uint64_t) curr_char - 'a' + 10;
		}

		curr_ind++;
	}

	kprintf("\n");

	uint64_t curr_multiple = 1;
	for (uint64_t backwards_ind = 0; (curr_ind - backwards_ind) >= 0; backwards_ind++) {
		addr += (uintptr_t) nums[curr_ind - backwards_ind] * curr_multiple;
		curr_multiple *= 16;
	}

	return addr;
}

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

