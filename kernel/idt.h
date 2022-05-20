#pragma once

#include <stdint.h>
#include <stddef.h>

#define SYS_write 1
#define SYS_read 0

// Reset v_heap
void reset_v_heap();

typedef struct interrupt_context {
  uintptr_t ip;
  uint64_t cs;
  uint64_t flags; // Change this to get in single instruction mode
  uintptr_t sp;
  uint64_t ss;
} __attribute__((packed)) interrupt_context_t;

__attribute__((interrupt))
void example_handler(interrupt_context_t* ctx);

__attribute__((interrupt))
void example_handler_ec(interrupt_context_t* ctx, uint64_t ec);

// A struct the matches the layout of an IDT entry
typedef struct idt_entry {
  uint16_t offset_0; // Points to handler ?? Why the fuck are there 3??
  uint16_t selector;
  uint8_t ist : 3;
  uint8_t _unused_0 : 5;
  uint8_t type : 4;
  uint8_t _unused_1 : 1;
  uint8_t dpl : 2;
  uint8_t present : 1;
  uint16_t offset_1;
  uint32_t offset_2;
  uint32_t _unused_2;
} __attribute__((packed)) idt_entry_t;

// Syscall to read in from stdin currently
int syscall_read(int fd, void* buf, size_t n);

/**
 * Set an interrupt handler for the given interrupt number.
 *
 * \param index The interrupt number to handle
 * \param fn    A pointer to the interrupt handler function
 * \param type  The type of interrupt handler being installed.
 *              Pass IDT_TYPE_INTERRUPT or IDT_TYPE_TRAP from above.
 */
void idt_set_handler(uint8_t index, void* fn, uint8_t type);

// This struct is used to load an IDT once we've set it up
typedef struct idt_record {
  uint16_t size;
  void* base;
} __attribute__((packed)) idt_record_t;

// We need to be able to perform syscalls
//   DON'T CALL WITH MORE THAN 6 ARGS
extern int syscall(uint64_t nr, ...);

extern void syscall_entry();

/**
 * Initialize an interrupt descriptor table, set handlers for standard exceptions, and install
 * the IDT.
 */
void idt_setup();
