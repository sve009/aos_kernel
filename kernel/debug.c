#include "debug.h"

#include "myio.h"

#include "kprint.h"
#include "elf.h"
#include "paging.h"
#include "pic.h"

#include <mystring.h>

#define ROWSIZE 6

int translate_hex_char(int hex_char) {
  if (hex_char >= '0' && hex_char <= '9') {
    return (int)(hex_char - '0');
  } else if (hex_char >= 'A' && hex_char <= 'F') {
    return (int)(hex_char - 'A' + 10);
  } else if (hex_char >= 'a' && hex_char <= 'f') {
    return (int)(hex_char - 'a' + 10);
  }
}


// Converts string address to usable addr
// Strings should be in the format 0x...
uintptr_t conv_addr(char* in) {
  int nums[16] = { 0 };

  // Loop through string backwards
  // Notably cuts off 0x
  for (int i = strlen(in) - 1; i >= 2; i--) {
    // Offset of 2 again
    nums[i - 2] = translate_hex_char(in[i]);  
  }

  // Keep track
  uintptr_t addr = 0;
	uint64_t curr_multiple = 1;

  // Now convert array of numbers to a single entry
	for (int i = strlen(in) - 4; i >= 0; i--) {
		addr += (uintptr_t) (nums[i] * curr_multiple);
		curr_multiple *= 16;
	}

  return addr;
}


// Reads address from user
uintptr_t get_addr() {
	uintptr_t addr = 0x0;
	uint64_t nums[16] = { 0 };
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
		if (curr_char >= '0' && curr_char <= '9') {
			nums[curr_ind] = (uint64_t) (curr_char - '0');
		} else if (curr_char >= 'A' && curr_char <= 'F') {
			nums[curr_ind] = (uint64_t) (curr_char - 'A' + 10);
		} else if (curr_char >= 'a' && curr_char <= 'f') {
			nums[curr_ind] = (uint64_t) (curr_char - 'a' + 10);
		}

		curr_ind++;
	}

  if (curr_ind == 16) {
    curr_ind = 15;
  }

	kprintf("\n");

	uint64_t curr_multiple = 1;
	for (int backwards_ind = curr_ind; backwards_ind >= 0; backwards_ind--) {
		addr += (uintptr_t) (nums[backwards_ind] * curr_multiple);
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

    // Set string table
    strtab = (char*)((uint64_t)ehdr + (uint64_t)shdr[ehdr->e_shstrndx].sh_offset);

    // Loop until both symbol + string tables found

    while (strtab == NULL || symtab == NULL) {

      // Crashes it basically immediately. What's going on here?
      /** if (shdr->sh_name != 0) { */
      /**   kprintf("Section: %s\n", &strtab[shdr->sh_name]); */
      /** } */

      // Symbol table
      if (shdr->sh_type  == 11) {
        // Set number of symbols
        //num_symbols = shdr->sh_size / (sizeof(Elf64_Sym));
        num_symbols = shdr->sh_size / shdr->sh_entsize;

        kprintf("%d Symbols found\n", num_symbols);

        // Set symtab
        symtab = (Elf64_Sym*)((uint64_t)ehdr + (uint64_t)shdr->sh_offset);
        temp = shdr;
      }

      // Increment header entry
      shdr++;
    }
  }
  /** kprintf("Hello\n"); */
  kprintf("strtab: %p\n", strtab);

  // Debug make sure string table works
  kprintf("Index: %d\n", temp->sh_name);
  kprintf("String at %p\n", &strtab[temp->sh_name]);
  kprintf("Name: %c\n", strtab[temp->sh_name]);
}

// Main loop
void debug_loop() {
  for (;;) {
    // Print a prompt
    kprintf("(debug) >>> ");

    // Read in the line to buf
    char buf[70] = { 0 };
    int i = 0;
    char curr;
    curr = kgetc(); 
    kprintf("%c", curr);

    while (curr != '\n') {
      buf[i] = curr;
      i++;
      curr = kgetc();
      kprintf("%c", curr);
    }

    buf[i++] = '\0';

    // Now process the line
    char* line_words[5] = { 0 };
    words(buf, line_words);

    // Handle commands
    if (line_words[0] == NULL) {
      // Error message
      kprintf("Valid commands are: [print] [continue]\n");
      // Do nothing
    } else if (my_strcmp("print", line_words[0]) == 0) {
      if (line_words[1] == NULL) {
        // Prompt for address
        uintptr_t ptr = get_addr();

        // Print out an int there
        print_int(ptr);
      } else if (my_strcmp("stack", line_words[1]) == 0) {
        // Dump the stack
        dump_stack();
      } else if (my_strcmp("string", line_words[1]) == 0) {
        // Print string case. Take next arg as an address
        if (line_words[2] == NULL) {
          kprintf("The next argument must be an address\n");
        } else {
          // Convert
          uintptr_t p = conv_addr(line_words[2]);
          // Print
          print_string((char*)p);
        }
      } else if (line_words[1][0] == '0') {
        // Get Address
        uintptr_t ptr = conv_addr(line_words[1]);

        // Print out ints worth for now
        print_int(ptr);
      } else {
        // TODO
        // Global lookups go here (hahhahahahahahahah)
        //
        // Prompt for address
        uintptr_t ptr = get_addr();

        // Print out an int there
        print_int(ptr);
      }
    } else if (my_strcmp("continue", line_words[0]) == 0) {
      return;
    } else {
      // Error message
      kprintf("Valid commands are: [print] [continue]\n");
    }
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
    kprintf("%s\n", sym_string);
    if (my_strcmp(symbol, sym_string) == 0) {
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
  uint8_t* p = (uint8_t*)start;

  // Iterate through rows
  for (int i = 0; i < rows; i++) {
    // Print address header
    kprintf("%p:", p + (2*cols*i));

    // Iterate through vals
    for (int j = 0; j < 2*cols; j++) {
      int val = *(p + (2*cols*i) + j);
      if (flag == HEXDUMP) {
        kprintf(" ");
        if (val <= 0xF) {
          kprintf("0");
        }
        kprintf("%x", val);
      } else if (flag == DECDUMP) {
        kprintf(" %d", val);
      } else {
        kprintf(" %c", val);
      }
    }

    // Break line
    kprintf("\n");
  }

}

// Shell function
void print_int(uint64_t p) {
  kprintf("%p: %d\n", p, *((int*)p));
}

// Another quick shell function
void print_string(char* s) {
  kprintf("len: %d\n", strlen(s));
  dump_mem(s, CHARDUMP, 1, strlen(s)/2);
}

// Use a quick hack to get stack pointer
// Notice that this is just an
//  estimate, as the sp isn't exact
void dump_stack() {
  // Add something to stack
  int dummy;

  // Get address
  uint64_t p = &dummy;

  // Dump
  dump_mem(p, HEXDUMP, 20, 10);
}

