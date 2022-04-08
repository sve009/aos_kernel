#pragma once

#include <stdint.h>

// Halt the CPU in an infinite loop
static void halt() {
  while (1) {
    __asm__("hlt");
  }
}

