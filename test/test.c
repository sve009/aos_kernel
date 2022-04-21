#include <myio.h>
#include <sys.h>

void _start() {
  // This file test out a bunch of things on the user side

  // Test printing
  char* greeting = "heya";
  printf("String: %s Int: %d Char: %c Hex: %x Pointer: %p\n", greeting, 10, 'a', 10, greeting);

  printf("Now we'll read in your next five characters: ");

  // Test reading
  // Read in five letter word
  char buf[6];
  my_read(0, buf, 5);
  buf[5] = '\0';

  printf("\nYou said: %s\n", buf);

  // Test getline
  // (Also tests malloc and program heap)
  printf("Now enter another line\n");
  char* line = getline();

  printf("Your line: %s", line);

  // Shell already tests execution.
  
  // Deliberately crash to demonstrate user mode
  printf("We will now crash to demonstrate we're in user mode\n");

  printf("First a message written to a test page mapped to be executable\n");

  // An executable page and nonexecutable page were set up by the exec function
  char* test_page = (char*)0x400000000; // this test page is writable
  test_page[0] = 'h';
  test_page[1] = 'e';
  test_page[2] = 'l';
  test_page[3] = 'l';
  test_page[4] = 'o';
  test_page[5] = '\n';
  my_write(1, test_page, 6);

  printf("Now a message written to a test page mapped to be nonwritable\n");
  
  // Set up to not have permissions
  char* test_page2 = (char*)0x400001800;
  test_page2[0] = 'h';
  test_page2[1] = 'e';
  test_page2[2] = 'l';
  test_page2[3] = 'l';
  test_page2[4] = 'o';
  test_page2[5] = '\n';
  my_write(1, test_page2, 6);

  // Shouldn't get here but we'll exit anyway
  my_exit();
}

