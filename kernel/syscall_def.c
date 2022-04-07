#include <stdint.h>
#include <stdbool.h>

#include "page.h"
#include "kprint.h"
#include "key.h"

#define BACKSPACE 8

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
    if (current == BACKSPACE) {
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
  // Check that fd is 1 or 2
  if (fd == 1 || fd == 2) {
    while (num_written < count) {
    kprint_c(cursor[num_written++]);
  }
    return num_written;
  }
  kprintf("write: invalid file descriptor provided, received %d\n", fd);
  return -1;
}

int64_t sys_mmap(void* addr, size_t length, int prot, int flags, int fd, uint16_t offset) {
  if (length <= 0) return -1;
  uintptr_t address_to_map;
  if (addr == NULL) {
    address_to_map = peek_freelist();
  } else {
    address_to_map = (uintptr_t) addr;
  }
  uint64_t size_left = length;
  int i = 0;
  do {
    // Allocate a requested page, set permissions to writable only
    if (vm_map(read_cr3(), address_to_map, 1, ((flags & 0x2) >> 1), ~(flags & 0x1) & 0x1) == false) {
      kprintf("mmap: failed to allocate memory for page %p\n", address_to_map);
      return -1;
    }
    if (addr == NULL) address_to_map = peek_freelist();
    if (size_left >= PAGE_SIZE) size_left -= PAGE_SIZE;
    i++;
  } while (size_left >= PAGE_SIZE);
  return address_to_map;
}

int sys_exec(char* name) {
  return 0;
}