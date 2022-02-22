#pragma once

// Halt the CPU in an infinite loop
static void halt() {
  while (1) {
    __asm__("hlt");
  }
}

// Useful for zeroing stuff out
static void memset(void* start, uint64_t size) {
  uint64_t progress = 0;

  uint8_t z = 0;
  
  while (progress < size) {
    *((uint8_t*)((uint64_t)start + progress)) = z;
    progress += sizeof(z);
  }
}
