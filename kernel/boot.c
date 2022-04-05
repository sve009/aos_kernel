#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "stivale2.h"
#include "util.h"
#include "kprint.h"
#include "idt.h"
#include "pic.h"
#include "paging.h"
#include "elf.h"
#include "term.h"


// Reserve space for the stack
static uint8_t stack[8192];

// Unmap page 0
static struct stivale2_tag unmap_null_hdr_tag = {
  .identifier = STIVALE2_HEADER_TAG_UNMAP_NULL_ID,
  .next = 0
};

// Request a terminal from the bootloader
static struct stivale2_header_tag_terminal terminal_hdr_tag = {
	.tag = {
    .identifier = STIVALE2_HEADER_TAG_TERMINAL_ID,
    .next = (uintptr_t)&unmap_null_hdr_tag
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

// Print out module locations
void print_mods(struct stivale2_struct* hdr) {
  struct stivale2_struct_tag_modules* tag = find_tag(hdr, 0x4b6fe466aade04ce);
  kprintf("Modules:\n");
  for (int i = 0; i < tag->module_count; i++) {
    kprintf("  %s 0x%x-0x%x\n", tag->modules[i].string, tag->modules[i].begin, tag->modules[i].end);
  }
}

typedef void call_t(void);

void execute_mods(struct stivale2_struct* hdr) {
  struct stivale2_struct_tag_modules* tag = find_tag(hdr, 0x4b6fe466aade04ce);
  kprintf("Modules:\n");
  for (int i = 0; i < tag->module_count; i++) {
    kprintf("  %s 0x%x-0x%x\n", tag->modules[i].string, tag->modules[i].begin, tag->modules[i].end);

    // Get header
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)tag->modules[i].begin;

    // Get program header
    Elf64_Phdr* phdr = (Elf64_Phdr*)((uintptr_t)ehdr + (uintptr_t)ehdr->e_phoff);

    // Table
    uintptr_t root = read_cr3() & 0xFFFFFFFFFFFFF000;

    // Load segments
    for (int i = 0; i < ehdr->e_phnum; i++) {
      // If type == LOAD
      // TODO: Might need multiple pages
      if (phdr[i].p_type == 1) {
        // Address:
        vm_map(root, (uintptr_t)phdr[i].p_vaddr, true, true, true);

        memcpy(phdr[i].p_vaddr, (void*)((uintptr_t)ehdr + (uintptr_t)phdr[i].p_offset), phdr[i].p_memsz);
      }
    }

    // Start executing
    call_t* entry = (call_t*)ehdr->e_entry; 
    entry();
  }
}

  


void term_setup(struct stivale2_struct* hdr) {
  // Look for a terminal tag
  struct stivale2_struct_tag_terminal* tag = find_tag(hdr, STIVALE2_STRUCT_TAG_TERMINAL_ID);

  // Make sure we find a terminal tag
  if (tag == NULL) halt();

  // Save the term_write function pointer
	set_term_write((term_write_t)tag->term_write);
}

/* 
 * Mapping functions
*/

uintptr_t hhdm_addr;

void memmap_print(struct stivale2_struct* hdr) {
  // Find memmap tag
  struct stivale2_struct_tag_memmap* tag = find_tag(hdr, STIVALE2_STRUCT_TAG_MEMMAP_ID);
  struct stivale2_struct_tag_hhdm* hhdm = find_tag(hdr, STIVALE2_STRUCT_TAG_HHDM_ID);

  hhdm_addr = hhdm->addr;
  
  // Make sure we actually found it
  if (tag == NULL) halt();
  if (hhdm == NULL) halt();

  kprintf("Usable Memory:\n");
  
  // Print off memory regions
  for (int i = 0; i < tag->entries; i++) {
    if (tag->memmap[i].type == 1) {
      kprintf("  %p-%p mapped at %p-%p\n", 
        tag->memmap[i].base, tag->memmap[i].base + tag->memmap[i].length, hhdm->addr + tag->memmap[i].base, hhdm->addr + tag->memmap[i].base + tag->memmap[i].length);
    }
  } 
}


void _start(struct stivale2_struct* hdr) {
  // We've booted! Let's start processing tags passed to use from the bootloader

  // Initialize free list (And hhdm tag)
  init_list(hdr);

  // Get page table location
  uintptr_t root = read_cr3() & 0xFFFFFFFFFFFFF000;

  // Set up terminal
  term_init();

  kprintf("Terminal initialized\n");

  kprintf("Setting up idt\n");

  // Set up idt
  idt_setup();

  // Unmap the lower half
  unmap_lower_half(root);

  // Update where we are

  kprintf("Initializing pic\n");

  // Initialize pic
  pic_init();
  
  // Unmask
  pic_unmask_irq(1);


  kprintf("Printing memory and mods\n");

  memmap_print(hdr);
  print_mods(hdr);



  // Get page table
  uintptr_t table = read_cr3();

  // Test translate
  //translate(table, _start);

  // Test vm map
  int* p = (int*)0x501040200;
  bool result = vm_map(root, (uintptr_t)p, true, true, true);
  if (result) {
    *p = 123;
    kprintf("Stored %d at %p\n", *p, p);
  } else {
    kprintf("vm_map failed with an error\n");
  }

  kprintf("Running executable\n");

  // Test running mods
  execute_mods(hdr);

  // Test syscall write
  syscall(SYS_write, 1, "Hello\n", 7);

  // Test syscall read
  char buf[6];
  long rc = syscall(SYS_read, 0, buf, 5);
  if (rc <= 0) {
    kprintf("read failed\n");
  } else {
    buf[rc] = '\0';
    kprintf("read '%s'\n", buf);
  }

  // Test buffer
  for (int i = 0; i < 600; i++) {
    char c = kgetc();
    kprintf("%c", c);
  }


  // Test exceptions
  //int* p = (int*)0x1;
  //*p = 123;

  //__asm__("int $3");

	// We're done, just hang...
	halt();
}
