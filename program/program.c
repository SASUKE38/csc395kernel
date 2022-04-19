#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <strlib.h>

void _start() {
  printf("in program\n");
  write(1, "Hello world!\n", 13);

  char* test = (char*) mmap(NULL, 0x1000 * 7 + 1, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  printf("test: %p\n", test);

  printf("Input to program: ");
  read(0, test, 5);
  printf("\n");
  test[6] = '\0';
  printf("program read: ");
  write(1, test, 5);
  printf("\n");

  exit(0);
}
