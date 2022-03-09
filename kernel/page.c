#include <stdint.h>
#include <stdbool.h>

#include "kprint.h"
#include "boot.h"
#include "strlib.h"
#include "page.h"

typedef struct freelist_node {
  struct freelist_node* next;
} freelist_node_t;

// Freelist of physical addresses
freelist_node_t* top = NULL;

typedef struct page_table_entry {
  bool present : 1;
  bool writable : 1;
  bool user : 1;
  uint16_t unused : 9;
  uint64_t address : 51;
  bool no_execute : 1;
} __attribute__((packed)) pt_entry_t;

uint64_t read_cr0() {
  uintptr_t value;
  __asm__("mov %%cr0, %0" : "=r" (value));
  return value;
}

void write_cr0(uint64_t value) {
  __asm__("mov %0, %%cr0" : : "r" (value));
}

uintptr_t read_cr3() {
  uintptr_t value;
  __asm__("mov %%cr3, %0" : "=r" (value));
  return value;
}

// This function was provided by Professor Curtsinger.
/**
 * Translate a virtual address to its mapped physical address
 *
 * \param address     The virtual address to translate
 */
void translate(void* address) {
  uintptr_t table_phys = read_cr3() & 0xFFFFFFFFFFFFF000;

  uintptr_t addr = (uintptr_t) address;
  uint16_t indices[] = {
    addr & 0xFFF,
    (addr >> 12) & 0x1FF,
    (addr >> 21) & 0x1FF,
    (addr >> 30) & 0x1FF,
    (addr >> 39) & 0x1FF,
  };

  pt_entry_t* table = (pt_entry_t*) phys_to_vir((void*)table_phys);//(table_phys + hhdm_base_global);
  kprintf("Translating %p\n",  address);
  /*kprintf("   page offset = %p\n",  indices[0]);
  kprintf("   index 1 = %p\n",  indices[1]);
  kprintf("   index 2 = %p\n",  indices[2]);
  kprintf("   index 3 = %p\n",  indices[3]);
  kprintf("   index 4 = %p\n",  indices[4]);*/

  for (int i = 4; i > 0; i--) {
    uint16_t index = indices[i];
    kprintf("table[index] at %p: %p\n", &table[index], table[index]);
    if (table[index].present == 1) {
      kprintf("%s", table[index].user ? "user" : "kernel");
      if (table[index].writable == 1) {
        kprintf("  writable");
      }
      if (table[index].no_execute == 0) {
        kprintf("  executable");
      }
      kprintf(" ->");

      table_phys = table[index].address << 12;
      kprintf(" %p\n", table_phys);
      table = (pt_entry_t*)phys_to_vir((void*)table_phys);
    } else {
      kprintf("  not present\n");
      return;
    }
  }
  kprintf("%p maps to %p\n", address, (table_phys + indices[0]));
}


void freelist_init(uint64_t* start, uint64_t* end, uint16_t num_sections) {
  uint64_t start_addr = *start;
  uint64_t end_addr = *end;
  // Loop until all memory sections are processed.
  for (int i = 0; i < num_sections; i++) {
    // Loop until all chunks of the current section are processed.
    while ((start_addr + PAGE_SIZE) <= end_addr) {
      pmem_free(start_addr);
      start_addr += PAGE_SIZE;
    }
    // Increment the position in the start and end arrays.
    start_addr = (uint64_t) start[i+1];
    end_addr = (uint64_t) end[i+1];
  }
}

/**
 * Allocate a page of physical memory.
 * \returns the physical address of the allocated physical memory or 0 on error.
 */
uintptr_t pmem_alloc() {
  if (top == NULL) return 0;
  freelist_node_t* vtop = phys_to_vir((void*) top);
  freelist_node_t* temp = top;
  top = vtop->next;
  return (uintptr_t) temp;
}

/**
 * Free a page of physical memory.
 * \param p is the physical address of the page to free, which must be page-aligned.
 */
void pmem_free(uintptr_t p) {
  if ((void*) p == NULL) {
    kprintf("pmem_free: attempted to free NULL\n");
    return;
  }
  if (p % PAGE_SIZE != 0) {
    kprintf("pmem_free: attempted to free a pointer that is not page-aligned\n");
    return;
  }
  freelist_node_t* new_node = (freelist_node_t*) p;
  freelist_node_t* vnew_node = phys_to_vir((void*) new_node);
  vnew_node->next = top;
  top = new_node;
}

// Print a specified number of elements of the freelist. For debugging
void print_freelist(int num_print) {
  freelist_node_t* cursor = top;
  for (int i = 0; i < num_print; i++) {
    kprintf("%p ", cursor);
    cursor = cursor->next;
  }
  kprintf("\n");
}

/**
 * Map a single page of memory into a virtual address space.
 * \param root The physical address of the top-level page table structure
 * \param address The virtual address to map into the address space, must be page-aligned
 * \param user Should the page be user-accessible?
 * \param writable Should the page be writable?
 * \param executable Should the page be executable?
 * \returns true if the mapping succeeded, or false if there was an error
 */
bool vm_map(uintptr_t root, uintptr_t address, bool user, bool writable, bool executable) {
  uintptr_t table_phys = root & 0xFFFFFFFFFFFFF000;

  uintptr_t addr = address;
  uint16_t indices[] = {
    addr & 0xFFF,
    (addr >> 12) & 0x1FF,
    (addr >> 21) & 0x1FF,
    (addr >> 30) & 0x1FF,
    (addr >> 39) & 0x1FF,
  };

  pt_entry_t* table = (pt_entry_t*) phys_to_vir((void*)table_phys);
  uintptr_t new_ptr;

  for (int i = 4; i > 0; i--) {
    uint16_t index = indices[i];
    // If the entry is present, move to the next level
    if (table[index].present == 1) {
      table_phys = table[index].address << 12;
      table = (pt_entry_t*) phys_to_vir((void*)table_phys);
    // Fill in the entry otherwise
    } else {
      // Get a pointer to a new page
      new_ptr = pmem_alloc();
      // Return false if the call to pmem_alloc failed
      if (new_ptr == 0) return false;
      // Zero out the new page
      memset((void*) phys_to_vir((void*)new_ptr), 0, PAGE_SIZE);
      // Set values
      table[index].present = 1;
      table[index].user = (i == 1 ? user : 1);
      table[index].writable = (i == 1 ? writable : 1);
      table[index].no_execute = (i == 1 ? executable : 0);
      table[index].address = new_ptr >> 12;

      // Return true if the last entry was filled in
      if (i == 1) return true;
      // Proceed to this new table
      table_phys = table[index].address << 12;
      table = (pt_entry_t*) phys_to_vir((void*)table_phys);
    }
  }
  // The last entry was present, so return false
  return false;
}

/**
 * Unmap a page from a virtual address space
 * \param root The physical address of the top-level page table structure
 * \param address The virtual address to unmap from the address space
 * \returns true if successful, or false if anything goes wrong
 */
bool vm_unmap(uintptr_t root, uintptr_t address) {
  uintptr_t table_phys = root & 0xFFFFFFFFFFFFF000;

  uintptr_t addr = (uintptr_t) address;
  uint16_t indices[] = {
    addr & 0xFFF,
    (addr >> 12) & 0x1FF,
    (addr >> 21) & 0x1FF,
    (addr >> 30) & 0x1FF,
    (addr >> 39) & 0x1FF,
  };

  pt_entry_t* table = (pt_entry_t*) phys_to_vir((void*)table_phys);
  uint16_t index = 0;
  uintptr_t to_free;
  for (int i = 4; i > 0; i--) {
    index = indices[i];
    kprintf("level: %d\n", i);
    kprintf("table[index].address in loop: %p\n", table[index].address);
    if (table[index].present == 0) return false;
    if (i == 1) to_free = table[index].address;
    table_phys = table[index].address << 12;
    table = (pt_entry_t*)phys_to_vir((void*)table_phys);
  }
  table[index].present = 0;
  kprintf("table[index].address after traversal: %p\n", to_free);
  pmem_free((uintptr_t)(to_free << 12));
  return true;
}

/**
 * Change the protections for a page in a virtual address space
 * \param root The physical address of the top-level page table structure
 * \param address The virtual address to update
 * \param user Should the page be user-accessible or kernel only?
 * \param writable Should the page be writable?
 * \param executable Should the page be executable?
 * \returns true if successful, or false if anything goes wrong (e.g. page is not mapped)
 */
bool vm_protect(uintptr_t root, uintptr_t address, bool user, bool writable, bool executable) {
  uintptr_t table_phys = root & 0xFFFFFFFFFFFFF000;

  uintptr_t addr = (uintptr_t) address;
  uint16_t indices[] = {
    addr & 0xFFF,
    (addr >> 12) & 0x1FF,
    (addr >> 21) & 0x1FF,
    (addr >> 30) & 0x1FF,
    (addr >> 39) & 0x1FF,
  };

  pt_entry_t* table = (pt_entry_t*) phys_to_vir((void*)table_phys);
  uint16_t index;

  for (int i = 4; i > 0; i--) {
    index = indices[i];
    if (table[index].present == 0) return false;

    if (i == 1) {
      table[index].user = user;
      table[index].writable = writable;
      table[index].no_execute = executable;
    }

    table_phys = table[index].address << 12;
    table = (pt_entry_t*)phys_to_vir((void*)table_phys);
  }
  return true;
}
