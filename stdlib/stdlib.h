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

/** Maps a new page into the virtual address space of the calling process.
* If addr is NULL, mmap chooses a page-aligned location to place the mapping.
* \param addr The desired address at which the mapping should begin.
* \param length The desired length of the mapping, in bytes. Rounded up to the next page internally.
* \param prot Permissions to associate with the mapping. Defined above.
* \param flags Flags to associate with the mapping. Disregarded in this simple implementation.
* \param fd Contents of the mapping to add. Disregarded in this simple implementation.
* \param offset Offset into the file at which the mapping should begin. Disregarded in this simple implementation.
* \returns A pointer to the start of the mapped region.
*/
void* mmap(void* addr, size_t length, int prot, int flags, int fd, uint16_t offset);

/** Allocates memory of a desired size.
* \param sz The size of the desired memory chunk.
* \returns A pointer to the allocated chunck of memory.
*/
void* malloc(size_t sz);

/** Frees a pointer allocated by malloc. In this implementation, actually does nothing.
* \param p The pointer to free.
*/
void free(void* p);

/**
 * Parses a string for an integer. Ignores initial whitespace.
 * \param nptr The string to parse.
 * \returns The parsed integer.
 */
int atoi(const char* nptr);

/** Terminates the calling process by loading the kernel's init program. Should be called by all processes
* that terminate using this kernel.
* \param ex The error code that the process exits with.
* \returns nothing in normal execution, or -1 if the internal system call failed.
*/
int64_t exit(uint64_t ex);
