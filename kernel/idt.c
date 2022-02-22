#include "idt.h"

#include <stdint.h>
#include <stddef.h>

#include "kprint.h"
#include "util.h"
#include "pic.h"
#include "port.h"

// Retrieved from:
//  https://stackoverflow.com/questions/61124564/convert-scancodes-to-ascii
char kbd_US [128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',   
  '\t', /* <-- Tab */
  'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',     
    0, /* <-- control key */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',  0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
  '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

__attribute__((interrupt))
void example_handler(interrupt_context_t* ctx) {
  kprintf("example interrupt handler\n");
  halt();
}

__attribute__((interrupt))
void example_handler_ec(interrupt_context_t* ctx, uint64_t ec) {
  kprintf("example interrupt handler (ec=%d)\n", ec);
  halt();
}

__attribute__((interrupt))
void divide_zero(interrupt_context_t* ctx, uint64_t ec) {
  kprintf("Divide by zero error (ec=%d)\n", ec);
  halt();
}

__attribute__((interrupt))
void segfault(interrupt_context_t* ctx, uint64_t ec) {
  kprintf("Segfault (ec=%d)\n", ec);
  halt();
}

__attribute__((interrupt))
void irq1_handler(interrupt_context_t* ctx) {
  // Get byte
  uint8_t input = inb(0x60);  

  // Output (and shift handling)
  if (!shift(input) && input < 127) { // Ordering is important here
    // Convert
    char val = kbd_US[input];

    // Add if shift mod
    if (is_shift() && val > 80) {
      val -= 32;
    } 

    // Write to buffer
    write_char(val);
  }


  // Acknowledge
  outb(PIC1_COMMAND, PIC_EOI);
}

// Every interrupt handler must specify a code selector. We'll use entry 5 (5*8=0x28), which
// is where our bootloader set up a usable code selector for 64-bit mode.
#define IDT_CODE_SELECTOR 0x28

// IDT entry types
#define IDT_TYPE_INTERRUPT 0xE
#define IDT_TYPE_TRAP 0xF

// Make an IDT
idt_entry_t idt[256];

/**
 * Set an interrupt handler for the given interrupt number.
 *
 * \param index The interrupt number to handle
 * \param fn    A pointer to the interrupt handler function
 * \param type  The type of interrupt handler being installed.
 *              Pass IDT_TYPE_INTERRUPT or IDT_TYPE_TRAP from above.
 */
void idt_set_handler(uint8_t index, void* fn, uint8_t type) {
  // Fill in all fields of idt[index]
  // Make sure you fill in:
  //   handler (all three parts, which requires some bit masking/shifting)
  //   type (using the parameter passed to this function)
  //   p=1 (the entry is present)
  //   dpl=0 (run the handler in kernel mode)
  //   ist=0 (we aren't using an interrupt stack table, so just pass 0)
  //   selector=IDT_CODE_SELECTOR

  // Make function uint64_t so it's *nice*
  uint64_t f = fn;

  // Zero (64 bit)
  uint64_t zero = 0;

  // Split handler into 3 parts:

  // Ordering is 2 1 0 

  // Part 0: 16 bits
  idt[index].offset_0 = (uint16_t)f; 
  
  // Part 1: 16 bits
  idt[index].offset_1 = (uint16_t)(f >> 16);

  // Part 2: 32 bits
  idt[index].offset_2 = (uint32_t)(f >> 32);

  // Type
  idt[index].type = type;

  // Present
  idt[index].present = 1;

  // DPL
  idt[index].dpl = 0;

  // IST
  idt[index].ist = 0;

  // Selector
  idt[index].selector = IDT_CODE_SELECTOR;
}


/**
 * Initialize an interrupt descriptor table, set handlers for standard exceptions, and install
 * the IDT.
 */
void idt_setup() {
  // Step 1: Zero out the IDT, probably using memset (which you'll have to implement)
  // Write me!

  kprintf("Setup\n");
  //memset(idt, sizeof(idt));


  // Step 2: Use idt_set_handler() to set handlers for the standard exceptions (1--21)
  // Write me!
  
  // Just use example for now

  for (int i = 0; i < 21; i++) {
    idt_set_handler(i, example_handler_ec, IDT_TYPE_INTERRUPT);
  }

  idt_set_handler(0, divide_zero, IDT_TYPE_INTERRUPT);
  idt_set_handler(3, example_handler, IDT_TYPE_INTERRUPT);
  idt_set_handler(11, segfault, IDT_TYPE_INTERRUPT);
  idt_set_handler(12, segfault, IDT_TYPE_INTERRUPT);
  idt_set_handler(13, segfault, IDT_TYPE_INTERRUPT);
  idt_set_handler(14, segfault, IDT_TYPE_INTERRUPT);
  idt_set_handler(IRQ1_INTERRUPT, irq1_handler, IDT_TYPE_INTERRUPT);

  kprintf("Handler: %p\n", example_handler_ec); // What the fuck?

  // Step 3: Install the IDT
  idt_record_t record = {
    .size = sizeof(idt),
    .base = idt
  };
  __asm__("lidt %0" :: "m"(record));
}
