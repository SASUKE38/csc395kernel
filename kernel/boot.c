#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <strlib.h>
#include <unistd.h>

#include <stdio.h>

#include "stivale2.h"
#include "util.h"

#include "kprint.h"
#include "idt.h"
#include "pic.h"
#include "key.h"
#include "page.h"
#include "syscall_def.h"
#include "elf.h"
#include "gdt.h"
#include "usermode_entry.h"
#include "loader.h"

#define MEMMAP_TAG_ID 0x2187f79e8612de07
#define HHDM_TAG_ID 0xb0ed257db18cb58f
#define MODULES_TAG_ID 0x4b6fe466aade04ce

#define MAX_MEM_SECTIONS 10

#define SYS_read 0
#define SYS_write 1

uint64_t hhdm_base_global;
struct stivale2_struct_tag_modules* modules_tag_global;

// Reserve space for the stack
static uint8_t stack[8192];

// Request 0x0 be unmapped
static struct stivale2_tag unmap_null_hdr_tag = {
  .identifier = STIVALE2_HEADER_TAG_UNMAP_NULL_ID,
  .next = 0
};

// Request a terminal from the bootloader
static struct stivale2_header_tag_terminal terminal_hdr_tag = {
	.tag = {
    .identifier = STIVALE2_HEADER_TAG_TERMINAL_ID,
    .next = (uintptr_t)&unmap_null_hdr_tag // Changed from 0; set the next tag in the list to be the unmap null tag(unmap_null_hdr_tag)
  },
  .flags = 0
};

// Declare the header for the bootloader
__attribute__((section(".stivale2hdr"), used))
static struct stivale2_header stivale_hdr = {
  // Use ELF file's default entry point
  .entry_point = 0,

  // Use stack (starting at the top)
  .stack = (uintptr_t)stack + sizeof(stack),

  // Bit 1: request pointers in the higher half
  // Bit 2: enable protected memory ranges (specified in PHDR)
  // Bit 3: virtual kernel mappings (no constraints on physical memory)
  // Bit 4: required
  .flags = 0x1E,
  
  // First tag struct
  .tags = (uintptr_t)&terminal_hdr_tag
};

// Find a tag with a given ID
void* find_tag(struct stivale2_struct* hdr, uint64_t id) {
  // Start at the first tag
	struct stivale2_tag* current = (struct stivale2_tag*)hdr->tags;

  // Loop as long as there are more tags to examine
	while (current != NULL) {
    // Does the current tag match?
		if (current->identifier == id) {
			return current;
		}

    // Move to the next tag
		current = (struct stivale2_tag*)current->next;
	}

  // No matching tag found
	return NULL;
}

/**
 * Prints memory usable by the kernel. 
 * \param hdr Pointer to the stivale2 header structure provided by the bootloader.
 */
void print_mem_address(struct stivale2_struct* hdr) {
  // Find the hhdm and memmap tags in the list
  struct stivale2_struct_tag_hhdm* hhdm_tag = find_tag(hdr, HHDM_TAG_ID);
  struct stivale2_struct_tag_memmap* memmap_tag = find_tag(hdr, MEMMAP_TAG_ID);
  uint64_t memmap_base;
  uint64_t memmap_end;
  uint64_t hhdm_addr = hhdm_tag->addr;

  kprintf("Usable Memory:\n");
  // Loop over the entries of the memmap
  for (int i = 0; i < memmap_tag->entries; i++) {
    // If the entry is usable, process it
    if (memmap_tag->memmap[i].type == 1) {
      // Find the start and end address of the usable range
      memmap_base = memmap_tag->memmap[i].base;
      memmap_end = memmap_base + memmap_tag->memmap[i].length;
      // Print the necessary values and conversions to virtual memory.
      kprintf("%p-%p mapped at %p-%p\n", (void*) memmap_base, (void*) memmap_end, 
        (void*) (memmap_base + hhdm_addr), (void*) (memmap_end + hhdm_addr));
    }
  }
}

/**
 * Fills an array with the start and end addresses of the usable memory regions.
 * The even indices are the start addresses while the odd indices are the end addresses.
 * Each pair of indices is one range.
 * \param hdr A pointer to the stivale2 header structure.
 * \param result The array to fill.
 * \returns The number of regions found.
 */
uint16_t get_mem_address(struct stivale2_struct* hdr, uint64_t* result) {
  // Find the hhdm and memmap tags in the list
  struct stivale2_struct_tag_memmap* memmap_tag = find_tag(hdr, MEMMAP_TAG_ID);
  uint64_t memmap_base;
  uint64_t memmap_end;
  uint16_t index = 0;

  // Loop over the entries of the memmap
  for (int i = 0; i < memmap_tag->entries; i++) {
    // If the entry is usable, process it
    if (memmap_tag->memmap[i].type == 1) {
      // Find the start and end address of the usable range
      memmap_base = memmap_tag->memmap[i].base;
      memmap_end = memmap_base + memmap_tag->memmap[i].length;
      result[index++] = memmap_base;
      result[index++] = memmap_end;
    }
  }
  return index;
}

/**
 * Converts a pointer representing a physical address to its virtual address.
 * \param ptr A pointer to be converted.
 * \returns The converted pointer.
 */
void* phys_to_vir(void* ptr) {
  return (void*) (hhdm_base_global + (uint64_t) ptr);
}

/**
 * Obtains the modules tag provided by the bootloader.
 * \returns A pointer to the modules tag.
 */
struct stivale2_struct_tag_modules* get_modules_tag() {
  return modules_tag_global;
}

/**
 * Handles system calls by choosing the correct function to process a given call.
 * The arguments passed must match those of the handler function.
 * \param nr The system call number.
 * \param arg0 The first argument to pass to the handler function.
 * \param arg1 The second argument to pass to the handler function.
 * \param arg2 The third argument to pass to the handler function.
 * \param arg3 The fourth argument to pass to the handler function.
 * \param arg4 The fifth argument to pass to the handler function.
 * \param arg5 The sixth argument to pass to the handler function.
 * \returns The value returned by the chosen handler function, or -1 if no function was chosen (invalid system call number)
 */
int64_t syscall_handler(uint64_t nr, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
  int64_t rc;
  // pick a system call
  switch(nr) {
    case 0: // read
      rc = sys_read(arg0, (void*) arg1, arg2);
      break;
    case 1: // write
      rc = sys_write(arg0, (void*) arg1, arg2);
      break;
    case 2: // mmap
      rc = sys_mmap((void*) arg0, arg1, arg2, arg3, arg4, arg5);
      break;
    case 3: // exec
      rc = sys_exec((char*) arg0);
      break;
    case 4: // exit
      rc = sys_exit(arg0);
      break;
    default:
      rc = -1;
      break;
  }
  return rc;
}

//extern int64_t syscall(uint64_t nr, ...);
// Assembly function to handle system calls. Calls syscall_handler.
extern void syscall_entry();

void _start(struct stivale2_struct* hdr) {
  // We've booted! Let's start processing tags passed to use from the bootloader
  // Find the hhdm tag
  struct stivale2_struct_tag_hhdm* hhdm_tag = find_tag(hdr, HHDM_TAG_ID);
  hhdm_base_global = hhdm_tag->addr;
  // Find the module tag
  //struct stivale2_struct_tag_modules* modules_tag = find_tag(hdr, MODULES_TAG_ID);
  modules_tag_global = find_tag(hdr, MODULES_TAG_ID);

  term_init();

  // Initialize PIC
  pic_init();
  // Initialize interrupt descriptor table
  idt_setup();

  // Initialize gdt to prepare to switch to user mode
  gdt_setup();

  pic_unmask_irq(1);
  // Set handler for system calls
  idt_set_handler(0x80, syscall_entry, IDT_TYPE_TRAP);

  // Print usable memory ranges
  print_mem_address(hdr);

  // Enable write protection
  uint64_t cr0 = read_cr0();
  cr0 |= 0x10000;
  write_cr0(cr0);

  // Freelist initialization
  uint64_t get_addr_result[MAX_MEM_SECTIONS];
  uint64_t start[MAX_MEM_SECTIONS];
  uint64_t end[MAX_MEM_SECTIONS];
  uint16_t num_read;

  // Fill array to pass to freelist_init
  num_read = get_mem_address(hdr, get_addr_result);
  // Separate start and end addresses
  int start_index = 0;
  int end_index = 0;
  for (int i = 0; i < num_read; i++) {
    if (i % 2 == 0) start[start_index++] = get_addr_result[i];
    else end[end_index++] = get_addr_result[i];
  }
  kprintf("Initializing freelist...\n");
  freelist_init(start, end, (num_read / 2));
  kprintf("Freelist initialized with %d sections.\n", (num_read / 2));
  unmap_lower_half(read_cr3() & 0xFFFFFFFFFFFFF000);
  // End freelist initialization

  /*int* test = (int*) mmap(NULL, 0x5000 + 1, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  kprintf("test: %p\n", test);

  *test = 3456;
  kprintf("Stored %d at %p\n", *test, test);

  int* test2 = (int*) mmap(NULL, 34, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  kprintf("test2: %p\n", test2);

  *test2 = 345678765;
  kprintf("Stored %d at %p\n", *test2, test2);*/

  /* PRINTING TESTS */
  //kprintf("char: %c\n", 'h');
  //kprintf("integer: %d\n", 45);
  //kprintf("hex: %x\n", 466);
  //kprintf("pointer: %p, hex: %x,int: %d, char: %c\n", 382743, 3847, 45, 'g');
  //kprintf("string: %s\n", "string");

  /* USER MODE TEST */
  //uintptr_t test_page = 0x400000000;
  //vm_map(read_cr3() & 0xFFFFFFFFFFFFF000, test_page, true, true, false);
  
  /* EXCEPTION TESTS */
  //__asm__("int $0");
  //__asm__("int $1");
  //__asm__("int $2");
  //__asm__("int $3");
  //__asm__("int $4");
  //__asm__("int $5");
  //__asm__("int $6");
  //__asm__("int $7");
  //__asm__("int $8");
  //__asm__("int $9");
  //__asm__("int $10");
  //__asm__("int $11");
  //__asm__("int $12");
  //__asm__("int $13");
  //__asm__("int $14");
  //__asm__("int $16");
  //__asm__("int $15");
  //__asm__("int $17");
  //__asm__("int $18");
  //__asm__("int $19");
  //__asm__("int $20");
  //__asm__("int $21");
  
  // Initialize the shell
  run_exec_elf("init", get_modules_tag());

  // Print error if loading init failed.
  kprintf("Failed to load init. Hanging.\n");

	// We're done, just hang...
	halt();
}
