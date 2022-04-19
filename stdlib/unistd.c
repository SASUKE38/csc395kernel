#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define SYS_read 0
#define SYS_write 1
#define SYS_exec 3

extern int64_t syscall(uint64_t nr, ...);

/**
* Reads characters from a specified file and places them in a buffer.
* 
* \param fd The file descriptor to read from. Should be 0.
* \param buf A pointer to store read characters in.
* \param count The number of character to read.
* \returns The number of characters read.
*/
int64_t read(int fd, void *buf, size_t count) {
  if (count == 0) return 0;
  return syscall(SYS_read, fd, buf, count);
}

/**
* Writes characters from a buffer to a specified file.
* 
* \param fd The file descriptor to read from. Should be 1 or 2.
* \param buf The buffer to write from.
* \param count The number of character to write.
* \returns The number of characters written.
*/
int64_t write(int fd, const void *buf, size_t count) {
  if (count == 0) return 0;
  return syscall(SYS_write, fd, buf, count);
}

/**
* Loads a process.
* 
* \param name The name of the process to load.
* \returns -1, since if this point is reached the process was not loaded.
*/
int64_t exec(char* name) {
  syscall(SYS_exec, name);
  return -1;
}
