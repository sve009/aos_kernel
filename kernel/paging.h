#pragma once

#include "stivale2.h"

#include <stdint.h>
#include <stdbool.h>

#define PAGE_SIZE 2048

// Read top level page table address
uintptr_t read_cr3();

// Write top level page table address
void write_cr3(uint64_t value);

// Get hhdm back
uintptr_t get_hhdm();

// Write to global var
void init_hhdm(intptr_t p);

// Conver physical to virtual address
uintptr_t ptov(uintptr_t v);

void translate(uintptr_t page_table, void* address);

// Unmap everything in the lower half of an address space with level 4 page table at address root
void unmap_lower_half(uintptr_t root);

// Initialize the free list by grabbing all available memory
void init_list(struct stivale2_struct* hdr);

/**
 * Allocate a page of physical memory.
 * \returns the physical address of the allocated physical memory or 0 on error.
 */
uintptr_t pmem_alloc();

/**
 * Free a page of physical memory.
 * \param p is the physical address of the page to free, which must be page-aligned.
 */
void pmem_free(uintptr_t p);

/**
 * Map a single page of memory into a virtual address space.
 * \param root The physical address of the top-level page table structure
 * \param address The virtual address to map into the address space, must be page-aligned
 * \param user Should the page be user-accessible?
 * \param writable Should the page be writable?
 * \param executable Should the page be executable?
 * \returns true if the mapping succeeded, or false if there was an error
 */
bool vm_map(uintptr_t root, uintptr_t address, bool user, bool writable, bool executable);

/**
 * Unmap a page from a virtual address space
 * \param root The physical address of the top-level page table structure
 * \param address The virtual address to unmap from the address space
 * \returns true if successful, or false if anything goes wrong
 */
bool vm_unmap(uintptr_t root, uintptr_t address);

/**
 * Change the protections for a page in a virtual address space
 * \param root The physical address of the top-level page table structure
 * \param address The virtual address to update
 * \param user Should the page be user-accessible or kernel only?
 * \param writable Should the page be writable?
 * \param executable Should the page be executable?
 * \returns true if successful, or false if anything goes wrong (e.g. page is not mapped)
 */
bool vm_protect(uintptr_t root, uintptr_t address, bool user, bool writable, bool executable);


