#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define SYS_read 0
#define SYS_write 1

extern int syscall(uint64_t nr, ...);

void _start() {

  syscall(SYS_write, 1, "Input to init:\n", 15);
  // Issue a read system call
  char buf[6];
  syscall(SYS_read, 0, buf, 5);

  // Write output
  syscall(SYS_write, 1, "read: ", 6);
  syscall(SYS_write, 1, buf, 5);
  syscall(SYS_write, 1, "\n", 1);
  syscall(SYS_write, 1, "returning from init\n", 20);

  // Loop forever
  //for(;;){}
}
