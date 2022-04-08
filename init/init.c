#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <myio.h>

extern int syscall(uint64_t nr, ...);

void _start() {
  

  // Test usermode access
  char* test_page = (char*)0x400000000;
  test_page[0] = 'h';
  test_page[1] = 'e';
  test_page[2] = 'l';
  test_page[3] = 'l';
  test_page[4] = 'o';
  test_page[5] = '\n';
  syscall(1, 1, test_page, 6);

  my_write(1, test_page, 6);


  // Issue a write system call
  syscall(1, 1, "Hello world!\n", 13);

  // Loop forever
  for(;;){}
}
