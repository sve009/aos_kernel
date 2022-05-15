#include "idt.h"

#include <stdint.h>
#include <stddef.h>

#include <mystring.h>

#include "kprint.h"
#include "util.h"
#include "pic.h"
#include "port.h"
#include "gdt.h"
#include "paging.h"
#include "proc.h"
#include "debug.h"

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
void divide_zero(interrupt_context_t* ctx) {
  kprintf("0: Divide by zero error\n");
  halt();
}

int i = 0;

__attribute__((interrupt))
void exception1(interrupt_context_t* ctx) {
  kprintf("1: Debug\n");
  kprintf("Called debug %d times", ++i);
  kprintf("Press anything to continue\n");
  //char c = kgetc();
  //kprintf("Char: %c\n");

  int test = 55;

  // Turn on single step
  kprintf("Flags: %p\n", ctx->flags);
  ctx->flags = ctx->flags | 0x100;
  kprintf("Flags: %p\n", ctx->flags);
}

__attribute__((interrupt))
void exception2(interrupt_context_t* ctx) {
  kprintf("2: Non-maskable interrupt\n");
  halt();
}

/**
 * This one will probably be important
 */
__attribute__((interrupt))
void exception3(interrupt_context_t* ctx) {
  kprintf("Pointer: %p\n", ctx->ip);
  kprintf("3: Breakpoint\n");
  kprintf("Press anything to continue\n");
  char c = kgetc();
  kprintf("Char: %c\n", c);

  int test = 55;

  //dump_mem(&test, HEXDUMP);
  print_int(&test);

  kprintf("Stack dump\n");
  dump_stack();

  // Turn on single step
  kprintf("Flags: %p\n", ctx->flags);
  ctx->flags = ctx->flags | 0x100;
  kprintf("Flags: %p\n", ctx->flags);
}

__attribute__((interrupt))
void exception4(interrupt_context_t* ctx) {
  kprintf("4: Overflow\n");
  halt();
}
__attribute__((interrupt))
void exception5(interrupt_context_t* ctx) {
  kprintf("5: Bound Range Exceeded\n");
  halt();
}
__attribute__((interrupt))
void exception6(interrupt_context_t* ctx) {
  kprintf("6: Invalid Opcode\n");
  halt();
}
__attribute__((interrupt))
void exception7(interrupt_context_t* ctx) {
  kprintf("7: Device not available\n");
  halt();
}

// the error code will always be zero
__attribute__((interrupt))
void exception8(interrupt_context_t* ctx, uint64_t ec) {
  kprintf("8: Double fault ec=%d\n", ec);
  halt();
}
__attribute__((interrupt))
void exception9(interrupt_context_t* ctx) {
  kprintf("9: Coprocessor segment overrun\n");
  halt();
}
__attribute__((interrupt))
void exception10(interrupt_context_t* ctx, uint64_t ec) {
  kprintf("10: Invalid TSS, ec=%d\n", ec); 
  
  // bit 1: external 
  if (ec & 0x0001) { kprintf("   There has been an external error\n"); }

  // get selector index: 
  int16_t index = ec >> 3; 

  // which table is error code from?
  ec = ec >> 1; 

  // bits 2-3: idt/gdt/ldt
  if (ec & 0x0000) { kprintf("   Problem occurred in the GDT at index %d\n", index); }
  if ((ec & 0x0001) || (ec & 0x0003)) { kprintf("    Problem occurred in the IDT at index %d\n", index); }
  if (ec & 0x0002) { kprintf("   Problem occurred in the LDT at index %d\n", index); }

  halt();
}

__attribute__((interrupt))
void exception11(interrupt_context_t* ctx, uint64_t ec) {
  kprintf("11: Segment not present, ec=%d\n", ec);

  // bit 1: external 
  if (ec & 0x0001) { kprintf("   There has been an external error\n"); }

  // get selector index: 
  int16_t index = ec >> 3; 

  // which table is error code from?
  ec = ec >> 1; 

  // bits 2-3: idt/gdt/ldt
  if (ec & 0x0000) { kprintf("   Problem occurred in the GDT at index %d\n", index); }
  if ((ec & 0x0001) || (ec & 0x0003)) { kprintf("    Problem occurred in the IDT at index %d\n", index); }
  if (ec & 0x0002) { kprintf("   Problem occurred in the LDT at index %d\n", index); }

  halt();
}

__attribute__((interrupt))
void exception12(interrupt_context_t* ctx, uint64_t ec) {
  kprintf("12: Stack-Segment fault, ec=%d\n", ec);

  // bit 1: external 
  if (ec & 0x0001) { kprintf("   There has been an external error\n"); }

  // get selector index: 
  int16_t index = ec >> 3; 

  // which table is error code from?
  ec = ec >> 1; 

  // bits 2-3: idt/gdt/ldt
  if (ec & 0x0000) { kprintf("   Problem occurred in the GDT at index %d\n", index); }
  if ((ec & 0x0001) || (ec & 0x0003)) { kprintf("    Problem occurred in the IDT at index %d\n", index); }
  if (ec & 0x0002) { kprintf("   Problem occurred in the LDT at index %d\n", index); }

  halt();
}

__attribute__((interrupt))
void exception13(interrupt_context_t* ctx, uint64_t ec) {
  kprintf("13: General protection fault, ec=%d\n", ec);

  // bit 1: external 
  if (ec & 0x0001) { kprintf("   There has been an external error\n"); }

  // get selector index: 
  int16_t index = ec >> 3; 

  // which table is error code from?
  ec = ec >> 1; 

  // bits 2-3: idt/gdt/ldt
  if (ec & 0x0000) { kprintf("   Problem occurred in the GDT at index %d\n", index); }
  if ((ec & 0x0001) || (ec & 0x0003)) { kprintf("    Problem occurred in the IDT at index %d\n", index); }
  if (ec & 0x0002) { kprintf("   Problem occurred in the LDT at index %d\n", index); }

  halt();
}

__attribute__((interrupt))
void exception16(interrupt_context_t* ctx) {
  kprintf("16: x87 Floating point exception\n");
  halt();
}

__attribute__((interrupt))
void exception17(interrupt_context_t* ctx, uint64_t ec) {
  kprintf("17: Alignment check, ec=%d\n", ec);
  kprintf("    Instruction which caused exception saved in pointer\n");
  halt();
}

__attribute__((interrupt))
void exception18(interrupt_context_t* ctx) {
  kprintf("18: Machine check\n");
  halt();
}

__attribute__((interrupt))
void exception19(interrupt_context_t* ctx) {
  kprintf("19: SIMD floating point exception\n");
  halt();
}

__attribute__((interrupt))
void exception20(interrupt_context_t* ctx) {
  kprintf("20: Virtualization exception\n");
  halt();
}

// no information on this from osdev wiki
__attribute__((interrupt))
void exception21(interrupt_context_t* ctx, uint64_t ec) {
  kprintf("21: Control protection exception, ec=%d\n", ec);
  halt();
}

__attribute__((interrupt))
void exception28(interrupt_context_t* ctx) {
  kprintf("28: Hypervisor injection exception\n");
  halt();
}

// no information on this from osdev wiki
__attribute__((interrupt))
void exception29(interrupt_context_t* ctx, uint64_t ec) {
  kprintf("29: VMM communication exception, ec=%d\n", ec);
  halt();
}

// no information on this from osdev wiki
__attribute__((interrupt))
void exception30(interrupt_context_t* ctx, uint64_t ec) {
  kprintf("30: Security exception, ec=%d\n", ec);
  halt();
}

// Get location of top-level page table
uintptr_t read_cr2() {
  uintptr_t value;
  __asm__("mov %%cr2, %0" : "=r" (value));
  return value;
}

__attribute__((interrupt))
void segfault(interrupt_context_t* ctx, uint64_t ec) {
  kprintf("cr2 = %p\n", read_cr2());
  kprintf("14: Page fault (ec=%d)\n", ec);

  // bit 1: present
  uint64_t bit = ec & 0x0001; 
  if (bit == 0) { kprintf("   Non-present page\n"); }
  else { kprintf("   Page protection violaton\n"); }

  // bit 2: write
  bit = ec & 0x0002; 
  if (bit == 0) { kprintf("   Caused by a read access\n"); }
  else { kprintf ("   Caused by a write access\n"); }

  // bit 3: user
  bit = ec & 0x0003;  
  if (bit == 1) { kprintf("   Fault happened in CPL=3\n"); }

  // bit 4: reserved writte
  bit = ec & 0x0004; 
  if (bit == 1) { kprintf("   One or more page directory entries contain reserved bits which are set to 1. This only applies when the PSE or PAE flags in CR4 are set to 1\n"); }

  // bit 5: instruction fetch
  bit = ec & 0x0005; 
  if (bit == 1) { kprintf("   Fault was caused by an instruction fetch. This only applies when the No-Execute bit is supported and enabled\n"); }


  // bit 6: protection key
  bit = ec & 0x0006;
  if (bit == 1) { kprintf("   Fault caused by a protection key violation\n"); }

  // bit 7: shadow stack
  bit = ec & 0x0007;
  if (bit == 1) { kprintf("   Fault caused by shadow stack access\n"); }

  // bit 15: software guard extensions
  bit = ec & 0x000f; 
  if (bit == 1) { kprintf("   Fault due to SGX violation - unrelated to paging\n"); }
  
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

// Read at kernel level
int syscall_read(int fd, void* buf, size_t n) {
  // Check fd
  if (fd != 0) {
    return -1;
  }

  // Count chars
  int accum = 0;

  while (accum < n) {
    // Read char
    char c = kgetc();

    // Write to buffer
    ((char*)buf)[accum++] = c;

    // Handle backspace
    if (c == '\b') {
      accum -= 2;
    }
  }

  return n;
}

// Write at kernel level
int syscall_write(int fd, char* buf, size_t n) {
  // If not stdout or stderr return
  if (fd != 1 && fd != 2) {
    return -1;
  }

  // Write from buffer
  for (int i = 0; i < n; i++) {
    kprintf("%c", buf[i]);
  }

  return n;
}

// Keep track of virtual address space heap
uint64_t v_heap = 0x10000;

void reset_v_heap() {
  v_heap = 0x10000;
}

int64_t syscall_handler(uint64_t nr, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
  // Read syscall
  if (nr == SYS_read) {
    return syscall_read(arg0, (void*)arg1, arg2);  
  }

  // Write syscall
  if (nr == SYS_write) {
    return syscall_write(arg0, (char*)arg1, arg2);  
  }

  // Mmap syscall
  if (nr == 2) {
    vm_map(read_cr3(), v_heap, 1, 1, 1);
    *(uint64_t*)arg0 = v_heap;
    v_heap += PAGE_SIZE;
    return 0;
  }

  // Exec syscall
  if (nr == 3) {
    // Copy the string over
    char copy[20] = { 0 };
    strcpy(copy, (char*)arg0); 
    // kprintf("copy: %s\n", copy);

    // Return if program not found
    if (check_mod(copy) < 0) {
      return -1;
    }

    // Otherwise execute
    unmap_lower_half(read_cr3());


    execute_mod(copy);    
    execute_mod("init");
  }

  // Exit syscall
  if (nr == 4) {
    kprintf("Never gets here\n");
    unmap_lower_half(read_cr3());
    execute_mod("init");    
  }

  return 123;
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
  idt[index].dpl = 3;

  // IST
  idt[index].ist = 0;

  // Selector
  idt[index].selector = KERNEL_CODE_SELECTOR;
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
  idt_set_handler(1, exception1, IDT_TYPE_INTERRUPT);
  idt_set_handler(2, exception2, IDT_TYPE_INTERRUPT);
  idt_set_handler(3, exception3, IDT_TYPE_TRAP);
  idt_set_handler(4, exception4, IDT_TYPE_TRAP);
  idt_set_handler(5, exception5, IDT_TYPE_INTERRUPT);
  idt_set_handler(6, exception6, IDT_TYPE_INTERRUPT);
  idt_set_handler(7, exception7, IDT_TYPE_INTERRUPT);
  idt_set_handler(8, exception8, IDT_TYPE_INTERRUPT);
  idt_set_handler(9, exception9, IDT_TYPE_INTERRUPT);
  idt_set_handler(10, exception10, IDT_TYPE_INTERRUPT);
  idt_set_handler(11, exception11, IDT_TYPE_INTERRUPT);
  idt_set_handler(12, exception12, IDT_TYPE_INTERRUPT);
  idt_set_handler(13, exception13, IDT_TYPE_INTERRUPT);
  idt_set_handler(14, segfault, IDT_TYPE_INTERRUPT);
  idt_set_handler(16, exception16, IDT_TYPE_INTERRUPT);
  idt_set_handler(17, exception17, IDT_TYPE_INTERRUPT);
  idt_set_handler(18, exception18, IDT_TYPE_INTERRUPT);
  idt_set_handler(19, exception19, IDT_TYPE_INTERRUPT);
  idt_set_handler(20, exception20, IDT_TYPE_INTERRUPT);
  idt_set_handler(21, exception21, IDT_TYPE_INTERRUPT);
  idt_set_handler(28, exception28, IDT_TYPE_INTERRUPT);
  idt_set_handler(29, exception29, IDT_TYPE_INTERRUPT);
  idt_set_handler(30, exception30, IDT_TYPE_INTERRUPT);

  idt_set_handler(IRQ1_INTERRUPT, irq1_handler, IDT_TYPE_INTERRUPT);
  idt_set_handler(0x80, syscall_entry, IDT_TYPE_TRAP); // syscalls

  kprintf("Handler: %p\n", example_handler_ec); // What the fuck?

  // Step 3: Install the IDT
  idt_record_t record = {
    .size = sizeof(idt),
    .base = idt
  };
  __asm__("lidt %0" :: "m"(record));
}
