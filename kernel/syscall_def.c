#include <stdint.h>
#include <strlib.h>
#include <stdbool.h>
#include <elf.h>

#include "page.h"
#include "kprint.h"
#include "key.h"
#include "loader.h"

#define BACKSPACE 8

// Stores location where mmap will map a page with no desired location.
uintptr_t mmap_next_start = 0x9000000000;

/**
* Reads characters from a specified file and places them in a buffer. Internal/system call version.
* 
* \param fd The file descriptor to read from. Should be 0.
* \param buf A pointer to store read characters in.
* \param count The number of character to read.
* \returns The number of characters read.
*/
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

/**
* Writes characters from a buffer to a specified file. Internal/system call version.
* 
* \param fd The file descriptor to read from. Should be 1 or 2.
* \param buf The buffer to write from.
* \param count The number of character to write.
* \returns The number of characters written.
*/
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

/** Maps a new page into the virtual address space of the calling process. Internal/system call version.
* If addr is NULL, mmap chooses a page-aligned location to place the mapping.
* \param addr The desired address at which the mapping should begin.
* \param length The desired length of the mapping, in bytes. Rounded up to the next page internally.
* \param prot Permissions to associate with the mapping. Defined above.
* \param flags Flags to associate with the mapping. Disregarded in this simple implementation.
* \param fd Contents of the mapping to add. Disregarded in this simple implementation.
* \param offset Offset into the file at which the mapping should begin. Disregarded in this simple implementation.
* \returns A pointer to the start of the mapped region.
*/
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

/**
* Loads a process. Internal/system call version.
* 
* \param name The name of the process to load.
* \returns -1, since if this point is reached the process was not loaded.
*/
int64_t sys_exec(char* name) {
  run_exec_elf(name, get_modules_tag());
  return -1;
}

/** Terminates the calling process by loading the kernel's init program. Should be called by all processes
* that terminate using this kernel. Internal/system call version.
* \param ex The error code that the process exits with.
* \returns nothing in normal execution, or -1 if the internal system call failed.
*/
int64_t sys_exit(uint64_t ex) {
  sys_exec("init");
  return -1;
}
