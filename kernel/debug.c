#include "debug.h"

#include "kprint.h"
#include "elf.h"

// #include <mystring.h>

#define ROWSIZE 6

// Keep track of elf string table + symbol table
char* strtab;
Elf64_Sym* symtab;

// Max number of symbols to look through
int num_symbols;

// Find + set tables
void init_tables(struct stivale2_struct_tag_modules* tag) {
  for (int i = 0; i < tag->module_count; i++) {
    // Find kernel mod
    if (my_strcmp(tag->modules[i].string, "kernel") != 0) {
      continue;
    }

    // Get header
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)tag->modules[i].begin;

    // Get section table
    Elf64_Shdr* shdr = (Elf64_Shdr*)((uint64_t)ehdr->e_shoff + (uint64_t)ehdr);

    // Set number of symbols
    num_symbols = shdr[2].sh_size / (sizeof(Elf64_Sym));

    // Set strtab and symtab
    symtab = (Elf64_Sym*)((uint64_t)ehdr + (uint64_t)shdr[2].sh_offset);
    strtab = (char*)((uint64_t)ehdr + (uint64_t)shdr[3].sh_offset);
  }
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

