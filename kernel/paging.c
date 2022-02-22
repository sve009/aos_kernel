#include "paging.h"

#include "kprint.h"
#include "util.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// hhdm
uintptr_t hhdm;

// Set hhdm
void init_hhdm(intptr_t p) {
  hhdm = p;
}



// Page table entry
typedef struct pte {
  bool present : 1;
  bool writable : 1;
  bool user : 1;
  uint16_t unused : 9;
  uint64_t address : 51;
  bool no_execute : 1;
} __attribute__((packed)) pte_t;

// Get location of top-level page table
uintptr_t read_cr3() {
  uintptr_t value;
  __asm__("mov %%cr3, %0" : "=r" (value));
  return value;
}

/**
 * Translate a virtual address to its mapped physical address
 *
 * \param address     The virtual address to translate
 */
void translate(uintptr_t page_table, void* address) {
  kprintf("Translating %p\n", address);

  uintptr_t offset = (uintptr_t)address & 0xfff;

  for (int i = 0; i < 4; i++) {
    // Get table offset
    uint16_t index = (uintptr_t) address >> (39 - 9*i);
    index = index & 0x1ff; // 9 bits

    // Print
    kprintf("Level %d (index %d of %p)\n", 4 - i, index, page_table);

    pte_t entry = ((pte_t*)page_table)[index];

    // Mask off bottom bits
    uintptr_t next = entry.address << 12;
    next = next & 0xFFFFFFFFFFFFF000;

    // Change table
    page_table = (hhdm + next);
  }

  // Update page table
  page_table = page_table - hhdm;

  // Look up final address
  kprintf("Final: %p\n", page_table + offset);
}

struct node {
  struct node* next;
};

struct node* free_list;

// Find a tag with a given ID
static void* find_tag(struct stivale2_struct* hdr, uint64_t id) {
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

// Initialize free list
void init_list(struct stivale2_struct* hdr) {
  // Find memmap tag
  struct stivale2_struct_tag_memmap* tag = find_tag(hdr, STIVALE2_STRUCT_TAG_MEMMAP_ID);
  struct stivale2_struct_tag_hhdm* hhdm_tag = find_tag(hdr, STIVALE2_STRUCT_TAG_HHDM_ID);

  int accum = 0;
  
  // Make sure we actually found it
  if (tag == NULL) halt();
  if (hhdm_tag == NULL) halt();

  // Set hhdm
  hhdm = hhdm_tag->addr;

  // Divy up memory regions
  for (int i = 0; i < tag->entries; i++) {
    if (tag->memmap[i].type == 1) {
      // Get size
      int n_pages = tag->memmap[i].length / 4096;

      // Keep track of which node we're on
      struct node* curr;

      // Add everything to free list
      for (int j = 0; j < n_pages; j++) {
        curr = (struct node*)(tag->memmap[i].base + (4096 * j));
        curr->next = free_list;
        free_list = (struct node*)((uintptr_t)curr + hhdm);

        accum++;
      }
    }
  } 

  kprintf("Initialized %d pages\n", accum);
}


uintptr_t pmem_alloc() {
  // Out of memory
  if (free_list == 0) {
    return 0; 
  }

  // Easy case
  struct node* fst = free_list;
  free_list = free_list->next;

  return (uintptr_t)fst;
}

// p physical
void pmem_free(uintptr_t p) {
  // Stick onto front of list
  struct node* n = (struct node*)p;
  n->next = free_list;
  free_list = n;
}

// Map some memory
bool vm_map(uintptr_t root, uintptr_t address, bool user, bool writable, bool executable) {
  // Ensure offset is 0
  address = address & 0xfffffffffffff000; 

  // Table indices
  int indices[] = {
    (address >> 12) & 0x1ff,
    (address >> 21) & 0x1ff,
    (address >> 30) & 0x1ff,
    (address >> 39) & 0x1ff
  };

  // Iterate through table levels
  for (int i = 3; i >= 0; i--) {
    // Fetch entry
    pte_t e = ((pte_t*)root)[indices[i]];

    // Special already present case
    if (i == 0 && e.present) {
      return false; // Already mapped
    }

    // Nonexistant case
    if (!e.present) {
      // Get new page + zero it
      
      uintptr_t p = pmem_alloc();
      memset((void*)p, 4096);

      // Update entry
      e.present = 1;
      e.user = 1;
      e.writable = 1;

      // Last level case
      if (i == 0) {
        // Set permissions
        e.writable = writable;
        e.user = user;
        e.no_execute = !executable;
      }

      // Either p or p >> 12
      e.address = p >> 12;
    }

    // Update root to new table
    //  (+ hhdm or no?)
    root = (e.address << 12) + hhdm;
  }

  // Now in the offset
  return true;
}

bool vm_unmap(uintptr_t root, uintptr_t address) {
  // Table indices
  int indices[] = {
    (address >> 12) & 0x1ff,
    (address >> 21) & 0x1ff,
    (address >> 30) & 0x1ff,
    (address >> 39) & 0x1ff
  };

  for (int i = 3; i >= 0; i++) {
    // Grab table entry
    pte_t entry = ((pte_t*)(root + hhdm))[indices[i]];

    // Special end case
    if (i == 0) {
      entry.present = false;
    }

    // Update root
    root = (entry.address << 12);
  }
  return true;
}

bool vm_protect(uintptr_t root, uintptr_t address, bool user, bool writable, bool executable) {
  // Table indices
  int indices[] = {
    (address >> 12) & 0x1ff,
    (address >> 21) & 0x1ff,
    (address >> 30) & 0x1ff,
    (address >> 39) & 0x1ff
  };

  for (int i = 3; i >= 0; i++) {
    // Grab table entry
    pte_t entry = ((pte_t*)(root + hhdm))[indices[i]];

    // Special end case
    if (i == 0) {
      // TODO: User
      entry.writable = writable;
      entry.user = user;
      entry.no_execute = !executable;
    }

    // Update root
    root = (entry.address << 12);
  }
  return true;
}
