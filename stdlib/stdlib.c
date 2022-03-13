#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

extern int syscall(uint64_t nr, ...);

void* mmap(void *addr, size_t length, int prot, int flags, int fd, uint16_t offset) {
  return syscall(SYS_mmap, addr, length, prot, flags, fd, offset);
}

// Round a value x up to the next multiple of y
#define ROUND_UP(x, y) ((x) % (y) == 0 ? (x) : (x) + ((y) - (x) % (y)))

void* bump = NULL;
size_t space_remaining = 0;

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
  while (isspace(*nptr) == 1) nptr++;
  for (int i = 0; nptr[i] != '\0'; i++) {
    if (isdigit(nptr[i]) == 1) {
      result *= 10;
      result += (nptr[i] - 48);
    } else break;
  }
  return result;
}
