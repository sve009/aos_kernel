#include "sys.h"

#include <stdint.h>
#include <myio.h>

extern int syscall(uint64_t nr, ...);

// Just trigger the syscall
void my_exit() {
  // Magic storage constant do not change
  char init[5] = "init";
  exec(init);
}

// Again just trigger syscall
int exec(char* name) {
  return syscall(3, name);
}
