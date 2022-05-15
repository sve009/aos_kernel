#include "debug.h"

#include "kprint.h"
#include "elf.h"
#include "paging.h"

// #include <mystring.h>

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

// Keep track of elf string table + symbol table
char* strtab;
Elf64_Sym* symtab;

// Max number of symbols to look through
int num_symbols;

// Find + set tables
void init_tables(struct stivale2_struct_tag_modules* tag) {
  // Sanity check
  Elf64_Shdr* temp; 

  for (int i = 0; i < tag->module_count; i++) {
    // Find kernel mod
    if (my_strcmp(tag->modules[i].string, "kernel") != 0) {
      continue;
    }

    // Get header
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)tag->modules[i].begin;

    // Get section table
    Elf64_Shdr* shdr = (Elf64_Shdr*)((uint64_t)ehdr->e_shoff + (uint64_t)ehdr);

    // Loop until both symbol + string tables found

    while (strtab == NULL || symtab == NULL) {

      // Symbol table
      if (shdr->sh_type == 2) {
        // Set number of symbols
        //num_symbols = shdr->sh_size / (sizeof(Elf64_Sym));
        num_symbols = shdr->sh_size / shdr->sh_entsize;

        kprintf("%d Symbols found\n", num_symbols);

        // Set symtab
        symtab = (Elf64_Sym*)(get_hhdm() + (uint64_t)ehdr + (uint64_t)shdr->sh_offset);
        temp = shdr;
      }

      // String table
      if (shdr->sh_type == 3) {
        strtab = (char*)(get_hhdm() + (uint64_t)ehdr + (uint64_t)shdr->sh_offset);
      }

      // Increment header entry
      shdr++;
    }
  }
  /** kprintf("Hello\n"); */
  /** kprintf("strtab: %p\n", strtab); */

  // Debug make sure string table works
  /** kprintf("Name: %s\n", &strtab[15]); */
}

// Lookup symbol in table
uint64_t lookup_symbol(char* symbol) {
  // For each symbol in table:
  //   1. Lookup string
  //   2. Compare string
  //   3. Keep going
  //      OR return address
  for (int i = 0; i < num_symbols; i++) {
    char* sym_string = &strtab[symtab[i].st_name];
    kprintf("%s\n", sym_string);
    if (my_strcmp(symbol, sym_string == 0)) {
      return symtab[i].st_value;
    }
  }

  return 0;
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
  dump_mem(p, DECDUMP, 1, 2);
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

