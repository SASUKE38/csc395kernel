#include <stddef.h>
#include <stdint.h>

#define SYS_read 0
#define SYS_write 1
#define SYS_exec 3

extern int64_t syscall(uint64_t nr, ...);

int64_t read(int fd, void *buf, size_t count) {
  if (count == 0) return 0;
  return syscall(SYS_read, fd, buf, count);
}

int64_t write(int fd, const void *buf, size_t count) {
  if (count == 0) return 0;
  return syscall(SYS_write, fd, buf, count);
}

int64_t exec(char* name) {
  return syscall(SYS_exec, name);
}
