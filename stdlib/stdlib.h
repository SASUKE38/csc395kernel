#pragma once

#include <stddef.h>

#define PAGE_SIZE 0x1000

#define SYS_mmap 2
#define SYS_exit 4

#define PROT_NONE 0x0
#define PROT_EXEC 0x1
#define PROT_WRITE 0x2
#define PROT_READ 0x4

#define MAP_ANONYMOUS 0x1
#define MAP_PRIVATE 0x2

void* mmap(void* addr, size_t length, int prot, int flags, int fd, uint16_t offset);

void* malloc(size_t sz);

void free(void* p);

/**
 * Parses a string for an integer. Ignores initial whitespace.
 * \param nptr The string to parse.
 * \returns The parsed integer.
 */
int atoi(const char* nptr);

int64_t exit(uint64_t ex);
