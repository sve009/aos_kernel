#include "paging.h"

#include "kprint.h"
#include "idt.h"
#include "util.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <mem.h>

// hhdm
uintptr_t hhdm;

// Set hhdm
void init_hhdm(intptr_t p) {
  hhdm = p;
}

// Reset cacheing when changing page table
void invalidate_tlb(uintptr_t virtual_address) {
   __asm__("invlpg (%0)" :: "r" (virtual_address) : "memory");
}

// Page table entry
typedef struct pte {
  bool present : 1;
  bool writable : 1;
  bool user : 1;
  bool write_through: 1;
  bool cache_disable: 1;
  bool accessed: 1;
  bool dirty: 1;
  bool page_size: 1;
  uint8_t unused : 4;
  uint64_t address : 51;
  bool no_execute : 1;
} __attribute__((packed)) pte_t;

// Get location of top-level page table
uintptr_t read_cr3() {
  uintptr_t value;
  __asm__("mov %%cr3, %0" : "=r" (value));
  return value;
}

// Write location of top-level page table
void write_cr3(uint64_t value) {
  __asm__("mov %0, %%cr3" : : "r" (value));
}

// Conver physical to virtual address
uintptr_t ptov(uintptr_t v) {
  return v + hhdm;
}


// Unmap everything in the lower half of an address space with level 4 page table at address root
void unmap_lower_half(uintptr_t root) {
  // We can reclaim memory used to hold page tables, but NOT the mapped pages
  pte_t* l4_table = (pte_t*)ptov(root);
  for (size_t l4_index = 0; l4_index < 256; l4_index++) {
    // Does this entry point to a level 3 table?
    if (l4_table[l4_index].present) {
      // Yes. Mark the entry as not present in the level 4 table
      l4_table[l4_index].present = false;

      // Now loop over the level 3 table
      pte_t* l3_table = (pte_t*)ptov(l4_table[l4_index].address << 12);
      for (size_t l3_index = 0; l3_index < 512; l3_index++) {

        // Does this entry point to a level 2 table?
        if (l3_table[l3_index].present && !l3_table[l3_index].page_size) {
          // Yes. Loop over the level 2 table
          pte_t* l2_table = (pte_t*)ptov(l3_table[l3_index].address << 12);
          for (size_t l2_index = 0; l2_index < 512; l2_index++) {

            // Does this entry point to a level 1 table?
            if (l2_table[l2_index].present && !l2_table[l2_index].page_size) {
              // Yes. Free the physical page the holds the level 1 table
              // SAM: Segfaults here (FIXED)
              pmem_free(l2_table[l2_index].address << 12);
            }
          }

          // Free the physical page that held the level 2 table
          pmem_free(l3_table[l3_index].address << 12);
        }
      }

      // Free the physical page that held the level 3 table
      pmem_free(l4_table[l4_index].address << 12);
    }
  }

  // Reset virtual heap pointer
  reset_v_heap();

  // Reload CR3 to flush any cached address translations
  write_cr3(read_cr3());
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

    pte_t entry = ((pte_t*)(page_table + hhdm))[index];

    // Mask off bottom bits
    uintptr_t next = entry.address << 12;
    next = next & 0xFFFFFFFFFFFFF000;

    // Change table
    page_table = next;
  }

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
  struct node* n = (struct node*)(p + hhdm);
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
    //   Oof ouch this line hurt me so much. I used pte_t e without &
    //   and updated it in memory without actually updating it
    //   and it took literally weeks to find
    //   and I am sad
    //   very sad
    pte_t* e = &((pte_t*)(root + hhdm))[indices[i]];

    // Special already present case
    if (i == 0 && e->present) {
      return false; // Already mapped
    }

    // Nonexistant case
    if (!e->present) {
      // Get new page + zero it
      uintptr_t p = pmem_alloc();
      memset((void*)p, 4096);

      // Update entry
      e->present = 1;
      e->user = 1;
      e->writable = 1;

      // Last level case
      if (i == 0) {
        // Set permissions
        e->writable = writable;
        e->user = user;
        e->no_execute = !executable;
      }

      // Either p or p >> 12
      e->address = (p - hhdm) >> 12;
    }

    // Update root to new table
    //  (+ hhdm or no?)
    root = (e->address << 12);
  }

  // Reset translation cacheing
  invalidate_tlb(address);

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

  for (int i = 3; i >= 0; i--) {
    // Grab table entry
    pte_t* entry = &((pte_t*)(root + hhdm))[indices[i]];

    // Special end case
    if (i == 0) {
      entry->present = false;
    }

    // Update root
    root = (entry->address << 12);
  }

  // Reset cacheing
  invalidate_tlb(address);

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

  for (int i = 3; i >= 0; i--) {
    // Grab table entry
    pte_t* e = &((pte_t*)(root + hhdm))[indices[i]];

    // If entry doesn't exist fail
    if (!e->present) {
      return false;
    }

    // Special end case
    if (i == 0) {
      // TODO: User
      e->writable = writable;
      e->user = user;
      e->no_execute = !executable;
    }

    // Update root
    root = (e->address << 12);
  }

  // Reset cacheing
  invalidate_tlb(address);

  return true;
}
