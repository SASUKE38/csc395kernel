#include <stdint.h>

#include "kprint.h"
#include "key.h"

int64_t sys_read(int16_t fd, void *buf, uint16_t count) {
  int64_t num_read = 0;
  char* result_buf = (char*) buf;
  // Check that fd is 0
  if (fd != 0) {
    kprintf("read: invalid file descriptor provided, received %d\n", fd);
    return -1;
  }
  char current;
  while (num_read < count) {
    // get a character
    current = kgetc();
    // handle backspace
    if (current == 8) {
      // do nothing if the buffer is empty
      if (num_read == 0) {
        continue;
      // else drop the most recently entered character
      } else {
        num_read--;
        result_buf[num_read] = '\0';
        continue;
      }
    }
    result_buf[num_read] = current;
    num_read++;
  }
  return num_read;
}

int64_t sys_write(int16_t fd, const void *buf, uint16_t count) {
  int64_t num_written = 0;
  char* cursor = (char*) buf;
  // Check that fd is 1
  if (fd == 1 || fd == 2) {
    while (num_written < count) {
    kprint_c(cursor[num_written++]);
  }
    return num_written;
  }
  kprintf("write: invalid file descriptor provided, received %d\n", fd);
  return -1;
}
