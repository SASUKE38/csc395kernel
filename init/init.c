#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define SYS_read 0
#define SYS_write 1

//extern int syscall(uint64_t nr, ...);

void _start() {
  //for (;;) {}
  /*syscall(SYS_write, 1, "Input to init:\n", 15);
  // Issue a read system call
  char buf[6];
  syscall(SYS_read, 0, buf, 5);

  // Write output
  syscall(SYS_write, 1, "read: ", 6);
  syscall(SYS_write, 1, buf, 5);
  syscall(SYS_write, 1, "\n", 1);
  syscall(SYS_write, 1, "returning from init\n", 20);*/

  char* test_page = (char*)0x400000000;
  test_page[0] = 'h';
  test_page[1] = 'e';
  test_page[2] = 'l';
  test_page[3] = 'l';
  test_page[4] = 'o';
  test_page[5] = '\n';
  write(1, test_page, 6);

  write(1, "Input to init:\n", 15);
  // Issue a read system call
  char buf[6];
  read(0, buf, 5);

  // Write output
  write(1, "read: ", 6);
  write(1, buf, 5);
  write(1, "\n", 1);
  printf("printf tests:\n");
  printf("char: %c\n", 'h');
  printf("integer: %d\n", 45);
  printf("hex: %x\n", 466);
  printf("pointer: %p\n", 382743);
  printf("string: %s\n", "string");
  write(1, "init finished\n", 14);

  //mmap(NULL, 3, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  // Loop forever
  for(;;){}
}
