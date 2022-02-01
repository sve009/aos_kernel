#include <stdint.h>
#include <stddef.h>

#include "stivale2.h"
#include "util.h"

// Reserve space for the stack
static uint8_t stack[8192];

// Request a terminal from the bootloader
static struct stivale2_header_tag_terminal terminal_hdr_tag = {
	.tag = {
    .identifier = STIVALE2_HEADER_TAG_TERMINAL_ID,
    .next = 0
  },
  .flags = 0
};

// Declare the header for the bootloader
__attribute__((section(".stivale2hdr"), used))
static struct stivale2_header stivale_hdr = {
  // Use ELF file's default entry point
  .entry_point = 0,

  // Use stack (starting at the top)
  .stack = (uintptr_t)stack + sizeof(stack),

  // Bit 1: request pointers in the higher half
  // Bit 2: enable protected memory ranges (specified in PHDR)
  // Bit 3: virtual kernel mappings (no constraints on physical memory)
  // Bit 4: required
  .flags = 0x1E,
  
  // First tag struct
  .tags = (uintptr_t)&terminal_hdr_tag
};

// Find a tag with a given ID
void* find_tag(struct stivale2_struct* hdr, uint64_t id) {
  // Start at the first tag
	struct stivale2_tag* current = (struct stivale2_tag*)hdr->tags;

  // Loop as long as there are more tags to examine
	while (current != NULL) {
    // Does the current tag match?
		if (current->identifier == id) {
			return current;
		}

    // Move to the next tag
		current = (struct stivale2_tag*)current->next;
	}

  // No matching tag found
	return NULL;
}

typedef void (*term_write_t)(const char*, size_t);
term_write_t term_write = NULL;

void term_setup(struct stivale2_struct* hdr) {
  // Look for a terminal tag
  struct stivale2_struct_tag_terminal* tag = find_tag(hdr, STIVALE2_STRUCT_TAG_TERMINAL_ID);

  // Make sure we find a terminal tag
  if (tag == NULL) halt();

  // Save the term_write function pointer
	term_write = (term_write_t)tag->term_write;
}

/*
   * Printing functions
 */

// Print one char
void kprint_c(char c) {
    term_write(&c, 1);
}

// Print string
void kprint_s(const char* str) {
    // Iterate through string and print each char
    while (*str != '\0') {
        kprint_c(*str);
        str += 1;
    }
}

// Print uint64
void kprint_d(uint64_t value) {
    // Store digit values
    char buffer[21] = { '\0' };
    int curr = 19;

    // Iterate through each digit.
    // Based off of:
    // https://stackoverflow.com/questions/3389264/how-to-get-the-separate-digits-of-an-int-number
    while (value > 0) {
        int d = value % 10;
        value /= 10;
        char c = d + 48; // 48 = 0 in ascii
        buffer[curr--] = c;
    }

    // Strip leading white spaces
    char* str = buffer;
    while (*str == '\0') {
        str++;
    }

    // Print
    kprint_s(str);
}

// Print hex
void kprint_x(uint64_t value) {
    // Store values
    char buffer[21] = { '\0' };
    int curr = 19;

    // Iterate through each digit:
    while (value > 0) {
        int d = value % 16;

        char c;

        // Convert to char
        if (d == 10) {
            c = 'A';
        } else if (d == 11) {
            c = 'B';
        } else if (d == 12) {
            c = 'C';
        } else if (d == 13) {
            c = 'D';
        } else if (d == 14) {
            c = 'E';
        } else if (d == 15) {
            c = 'F';
        } else {
            c = d + 48; // Again add for ascii
        }

        // Divide d
        value /= 16;

        // Add to buffer
        buffer[curr--] = c;
    }

    // Strip whitespace
    char* str = buffer;
    while (*str == '\0') {
        str++;
    }

    // Print
    kprint_s(str);
}

// Print pointer
void kprint_p(void* ptr) {
    kprint_s("0x");
    kprint_x((uint64_t)ptr);
}

void _start(struct stivale2_struct* hdr) {
  // We've booted! Let's start processing tags passed to use from the bootloader
  term_setup(hdr);

  // Print a greeting
  term_write("Hello Kernel!\n", 14);

  char c;

  kprint_d(12345);
  kprint_c('\n');
  kprint_p(&c);

	// We're done, just hang...
	halt();
}
