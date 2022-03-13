#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>

#define SYS_read 0
#define SYS_write 1

//extern int syscall(uint64_t nr, ...);

void _start() {

  /*syscall(SYS_write, 1, "Input to init:\n", 15);
  // Issue a read system call
  char buf[6];
  syscall(SYS_read, 0, buf, 5);

  // Write output
  syscall(SYS_write, 1, "read: ", 6);
  syscall(SYS_write, 1, buf, 5);
  syscall(SYS_write, 1, "\n", 1);
  syscall(SYS_write, 1, "returning from init\n", 20);*/

  write(1, "Input to init:\n", 15);
  // Issue a read system call
  char buf[6];
  read(0, buf, 5);

  // Write output
  write(1, "read: ", 6);
  write(1, buf, 5);
  write(1, "\n", 1);
  write(1, "returning from init\n", 20);

  mmap(NULL, 3, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  // Loop forever
  //for(;;){}
}
