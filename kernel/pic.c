#include "pic.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "port.h"
#include "kprint.h"
#include "debug.h"

#define ICW1_ICW4 0x01
#define ICW1_SINGLE 0x02
#define ICW1_INTERVAL4 0x04
#define ICW1_LEVEL 0x08
#define ICW1_INIT 0x10
 
#define ICW4_8086 0x01
#define ICW4_AUTO 0x02
#define ICW4_BUF_PIC2 0x08
#define ICW4_BUF_PIC1 0x0C
#define ICW4_SFNM 0x10

#define BUFFER_LEN 500

int readhead = 0;
int writehead = 0;
char buffer[BUFFER_LEN] = { 0 };

// Global mods
bool shiftl = false;
bool shiftr = false;

volatile uint16_t avail = 0;

// Shifting
bool shift(uint8_t in) {
  // lshift down
  if (in == 0x2A) {
    shiftl = true;
    return true;
  }
  // rshift down
  if (in == 0x36) {
    shiftr = true;
    return true;
  }
  // lshift up
  if (in == 0xAA) {
    shiftl = false;
    return true;
  }
  // rshift up
  if (in == 0xB6) {
    shiftr= false;
    return true;
  }
  return false;
}

// Get shift state
bool is_shift() {
  return shiftl || shiftr;
}

// Write to the buffer
void write_char(char in) {
  // Maxed out?
  if (readhead == writehead && avail != 0) {
    return;
  }

  // Store
  buffer[writehead++] = in;

  // Update avail
  avail++;

  // Rotation case
  writehead = writehead % BUFFER_LEN;
}

// Read from buffer
char kgetc() {
  // Unwritten case
  while (avail == 0) {}

  // Normal case
  char c = buffer[readhead];
  buffer[readhead++] = 0;
  readhead = readhead % BUFFER_LEN;

  // Update avail
  avail--;

  return c;
}


/**
 * Initialize the PICs to pass IRQs starting at 0x20
 * Based on code from https://wiki.osdev.org/PIC
 */
void pic_init() {
  // Start initializing PICs
  outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
  io_wait();
  outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
  io_wait();

  // Set offset for primary PIC
  outb(PIC1_DATA, IRQ0_INTERRUPT);
  io_wait();

  // Set offset for secondary PIC
  outb(PIC2_DATA, IRQ8_INTERRUPT);
  io_wait();

  // Tell primary PIC there is a secondary at IRQ2
  outb(PIC1_DATA, 0x04);
  io_wait();

  // Tell secondary PIC its identity
  outb(PIC2_DATA, 0x02);
  io_wait();
 
  // Finish initialization
  outb(PIC1_DATA, ICW4_8086);
  io_wait();
  outb(PIC2_DATA, ICW4_8086);
  io_wait();

  // Mask all IRQs by default
  outb(PIC1_DATA, 0xFF);
  outb(PIC2_DATA, 0xFF);

  // Enable interrupts
  __asm__("sti");
}

/// Mask an IRQ by number (0-15)
void pic_mask_irq(uint8_t num) {
  // Which PIC do we need to talk to?
  if (num < 8) {
    // Get the current mask
    uint8_t mask = inb(PIC1_DATA);

    // Update the mask
    mask |= 1<<num;
    outb(PIC1_DATA, mask);

  } else if (num < 16) {
    // Get the current mask
    uint8_t mask = inb(PIC2_DATA);

    // Update the mask
    mask |= 1<<(num - 8);
    outb(PIC2_DATA, mask);
  }
}

/// Unmask an IRQ by number (0-15)
void pic_unmask_irq(uint8_t num) {
  // Which PIC do we need to talk to?
  if (num < 8) {
    // Get the current mask
    uint8_t mask = inb(PIC1_DATA);

    // Update the mask
    mask &= ~(1<<num);
    outb(PIC1_DATA, mask);

  } else if (num < 16) {
    // Get the current mask
    uint8_t mask = inb(PIC2_DATA);

    // Update the mask
    mask &= ~(1<<(num - 8));
    outb(PIC2_DATA, mask);
  }
}

