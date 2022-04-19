#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <strlib.h>

void _start() {
  write(1, "Hello world!\n", 13);

  // Test mmap
  char* test = (char*) mmap(NULL, 0x1000 * 7 + 1, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  printf("pointer obtained from mmap: %p\n", test);

  // Test writing to memory obtained from mmap
  printf("Input to program: ");
  read(0, test, 5);
  printf("\n");
  test[6] = '\0';
  // Echo input to terminal
  printf("program read: ");
  write(1, test, 5);
  printf("\n");

  exit(0);
}
