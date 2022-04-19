#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>

extern int64_t syscall(uint64_t nr, ...);

/** Maps a new page into the virtual address space of the calling process.
* If addr is NULL, mmap chooses a page-aligned location to place the mapping.
* \param addr The desired address at which the mapping should begin.
* \param length The desired length of the mapping, in bytes. Rounded up to the next page internally.
* \param prot Permissions to associate with the mapping. Defined in stdlib.h.
* \param flags Flags to associate with the mapping. Disregarded in this simple implementation.
* \param fd Contents of the mapping to add. Disregarded in this simple implementation.
* \param offset Offset into the file at which the mapping should begin. Disregarded in this simple implementation.
* \returns A pointer to the start of the mapped region.
*/
void* mmap(void *addr, size_t length, int prot, int flags, int fd, uint16_t offset) {
  return (void*) syscall(SYS_mmap, addr, length, prot, flags, fd, offset);
}

// Round a value x up to the next multiple of y
#define ROUND_UP(x, y) ((x) % (y) == 0 ? (x) : (x) + ((y) - (x) % (y)))

void* bump = NULL;
size_t space_remaining = 0;

/** Allocates memory of a desired size.
* \param sz The size of the desired memory chunk.
* \returns A pointer to the allocated chunck of memory.
*/
void* malloc(size_t sz) {
  // Round sz up to a multiple of 16
  sz = ROUND_UP(sz, 16);

  // Do we have enough space to satisfy this allocation?
  if (space_remaining < sz) {
    // No. Get some more space using `mmap`
    size_t rounded_up = ROUND_UP(sz, PAGE_SIZE);
    void* newmem = mmap(NULL, rounded_up, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    // Check for errors
    if (newmem == NULL) {
      return NULL;
    }

    bump = newmem;
    space_remaining = rounded_up;
  }

  // Grab bytes from the beginning of our bump pointer region
  void* result = bump;
  bump += sz;
  space_remaining -= sz;

  return result;
}

/** Frees a pointer allocated by malloc. In this implementation, actually does nothing.
* \param p The pointer to free.
*/
void free(void* p) {
  // Do nothing
}

/**
 * Parses a string for an integer. Ignores initial whitespace.
 * \param nptr The string to parse.
 * \returns The parsed integer.
 */
int atoi(const char *nptr) {
  int result = 0;
  // Skip whitespace.
  while (isspace(*nptr) == 1) nptr++;
  for (int i = 0; nptr[i] != '\0'; i++) {
    // If the current character is a number, add it to the parsed integer.
    if (isdigit(nptr[i]) == 1) {
      // Shift the digits to make room for the new number.
      result *= 10;
      // Convert the character to the proper number using its ASCII code.
      result += (nptr[i] - 48);
    // End when a full number has been extracted.
    } else break;
  }
  return result;
}

/** Terminates the calling process by loading the kernel's init program. Should be called by all processes
* that terminate using this kernel.
* \param ex The error code that the process exits with.
* \returns nothing in normal execution, or -1 if the internal system call failed.
*/
int64_t exit(uint64_t ex) {
  syscall(SYS_exit, ex);
  return -1;
}
