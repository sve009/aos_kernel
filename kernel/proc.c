#include "proc.h"

#include <mystring.h>
#include <mem.h>

#include "kprint.h"
#include "elf.h"
#include "paging.h"
#include "usermode_entry.h"
#include "gdt.h"

struct stivale2_struct_tag_modules* tag;

void set_hdr(struct stivale2_struct_tag_modules* hdr) {
  tag = hdr;
}

void execute_mod(char* mod) {
  kprintf("Modules:\n");
  for (int i = 0; i < tag->module_count; i++) {
    kprintf("  %s 0x%x-0x%x\n", tag->modules[i].string, tag->modules[i].begin, tag->modules[i].end);

    if (my_strcmp(tag->modules[i].string, mod) != 0) {
      continue;
    }

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

    kprintf("Starting to jump into user mode\n");

    // Pick an arbitrary location and size for the user-mode stack
    uintptr_t user_stack = 0x70000000000;
    size_t user_stack_size = 8 * PAGE_SIZE;

    // Map the user-mode-stack
    for(uintptr_t p = user_stack; p < user_stack + user_stack_size; p += 0x1000) {
      // Map a page that is user-accessible, writable, but not executable
      vm_map(read_cr3() & 0xFFFFFFFFFFFFF000, p, true, true, false);
    }

    kprintf("Allocated space\n");

    // Map a page for testing
    uintptr_t test_page = 0x400000000;
    vm_map(read_cr3() & 0xFFFFFFFFFFFFF000, test_page, true, true, false);
    // vm_map(read_cr3() & 0xFFFFFFFFFFFFF000, test_page, false, true, false);

    // And now jump to the entry point
    usermode_entry(USER_DATA_SELECTOR | 0x3,            // User data selector with priv=3
                    user_stack + user_stack_size - 8,   // Stack starts at the high address minus 8 bytes
                    USER_CODE_SELECTOR | 0x3,           // User code selector with priv=3
                    ehdr->e_entry);                     // Jump to the entry point specified in the ELF file
  }
}
