#include <stdint.h>
#include <strlib.h>
#include <stdbool.h>

#include "page.h"
#include "kprint.h"
#include "key.h"
#include "loader.h"
#include "elf.h"

#define BACKSPACE 8

uintptr_t mmap_next_start = 0x9000000000;

int64_t sys_read(int16_t fd, void *buf, uint16_t count) {
  int64_t num_read = 0;
  char* result_buf = (char*) buf;
  // Check that fd is 0
  if (fd != 0) {
    // Return -1 if an invalid file descriptor was provided
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
  // Return -1 if an invalid file descriptor was provided
  return -1;
}

int64_t sys_mmap(void* addr, size_t length, int prot, int flags, int fd, uint16_t offset) {
  if (length <= 0) return -1; // length must be greater than 0
  uintptr_t address_to_map;
  if (addr == NULL) {
    address_to_map = mmap_next_start;
  } else {
    address_to_map = (uintptr_t) addr;
  }
  uintptr_t result = address_to_map;
  int64_t size_left = length;
  do {
    //kprintf("loop, address_to_map: %p\n", address_to_map);
    // Allocate a requested page, set permissions to requested permissions
    if (vm_map(read_cr3(), address_to_map, 1, ((flags & 0x2) >> 1), ~(flags & 0x1) & 0x1) == false) {
      // Return -1 if the mapping failed
      return -1;
    }
    // Advance to the next possible page to allocate
    mmap_next_start += PAGE_SIZE;
    if (addr == NULL) address_to_map = mmap_next_start;
    size_left -= PAGE_SIZE;
  } while (size_left > 0);
  return result;
}

int64_t sys_exec(char* name) {
  run_exec_elf(name, get_modules_tag());
  return -1;
}

int64_t sys_exit(uint64_t ex) {
  sys_exec("init");
  return -1;
}
