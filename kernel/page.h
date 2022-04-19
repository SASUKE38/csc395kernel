#pragma once

#include <stdbool.h>

#define PAGE_SIZE 0x1000

/**
 * Print a selected number of items on the freelist.
 *
 * \param num_print The number of entries to print
 */
void print_freelist(int num_print);

/** Reads the value of the cr0 register.
* \returns The value of the cr0 register.
*/
uint64_t read_cr0();

void write_cr0(uint64_t value);

/**
 * Obtain a pointer to the top-level page structure.
 * \returns a pointer to the top-level page structure.
 */
uintptr_t read_cr3();

/** Writes a value to the cr3 register.
* \param value The value to write
*/
void write_cr3(uint64_t value);

/**
 * This function unmaps everything in the lower half of an address space with level 4 page table at address root.
 *
 * \param root     Pointer to the top-level page structure
 */
void unmap_lower_half(uintptr_t root);

uintptr_t peek_freelist();

/**
 * Translate a virtual address to its mapped physical address
 *
 * \param address     The virtual address to translate
 */
void translate(void* address);

/**
 * Initializes the system's freelist. 
 * Takes an array of memory sections and adds all the pages in that section to the freelist.
 *
 * \param start Pointer that is the start of the current memory section.
 * \param end Pointer that is the end of the current memory section.
 * \param num_sections The number of memory sections to process.
 */
void freelist_init(uint64_t* start, uint64_t* end, uint16_t num_sections);
/**
 * Allocate a page of physical memory.
 * \returns the physical address of the allocated physical memory or 0 on error.
 */
uintptr_t pmem_alloc();

/**
 * Free a page of physical memory.
 * \param p is the physical address of the page to free, which must be page-aligned.
 */
void pmem_free(uintptr_t p);

/**
 * Map a single page of memory into a virtual address space.
 * \param root The physical address of the top-level page table structure
 * \param address The virtual address to map into the address space, must be page-aligned
 * \param user Should the page be user-accessible?
 * \param writable Should the page be writable?
 * \param executable Should the page be executable?
 * \returns true if the mapping succeeded, or false if there was an error
 */
bool vm_map(uintptr_t root, uintptr_t address, bool user, bool writable, bool executable);

/**
 * Unmap a page from a virtual address space
 * \param root The physical address of the top-level page table structure
 * \param address The virtual address to unmap from the address space
 * \returns true if successful, or false if anything goes wrong
 */
bool vm_unmap(uintptr_t root, uintptr_t address);

/**
 * Change the protections for a page in a virtual address space
 * \param root The physical address of the top-level page table structure
 * \param address The virtual address to update
 * \param user Should the page be user-accessible or kernel only?
 * \param writable Should the page be writable?
 * \param executable Should the page be executable?
 * \returns true if successful, or false if anything goes wrong (e.g. page is not mapped)
 */
bool vm_protect(uintptr_t root, uintptr_t address, bool user, bool writable, bool executable);
