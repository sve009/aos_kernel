#include <myio.h>
#include <sys.h>

extern int syscall(uint64_t nr, ...);

void _start() {
  printf("Hello world!\n");
  printf("This has been hello world\n");

  my_exit();
  
  // I truly don't understand
  // my_exit literally does the same thing
  //   but it page faults and this works so ??
  /** if (exec("init") == -1) { */
  /**   printf("Couldn't find mod\n"); */
  /** } */
}
