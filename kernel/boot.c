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

// Print usable memory using memmap and hhdm tags
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

void* phys_to_vir(void* ptr) {
  return (void*) (hhdm_base_global + (uint64_t) ptr);
}

struct stivale2_struct_tag_modules* get_modules_tag() {
  return modules_tag_global;
}

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
    default:
      rc = -1;
      break;
  }
  return rc;
}

extern int64_t syscall(uint64_t nr, ...);
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

  uintptr_t root = read_cr3() & 0xFFFFFFFFFFFFF000;
  int* p = (int*)peek_freelist();
  bool result = vm_map(root, (uintptr_t)p, false, true, false);
  if (result) {
    *p = 123;
    kprintf("Stored %d at %p\n", *p, p);
  } else {
    kprintf("vm_map failed with an error\n");
  }

  /*int* test = (int*) mmap(NULL, PAGE_SIZE * 7 + 1, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  kprintf("test: %p\n", test);

  *test = 3456;
  kprintf("Stored %d at %p\n", *test, test);

  int* test2 = (int*) mmap(NULL, 34, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  kprintf("test2: %p\n", test2);

  *test2 = 345678765;
  kprintf("Stored %d at %p\n", *test2, test2);*/

  uintptr_t test_page = 0x400000000;
  vm_map(read_cr3() & 0xFFFFFFFFFFFFF000, test_page, true, true, false);
  write(1, "kernel\n", 7);

  char* line_test = NULL;
  size_t line_size = 0;
  kprintf("input to line test: \n");
  getline(&line_test, &line_size);
  kprintf("read: %s\n", line_test);

  // Process modules
  //run_exec_elf("init", modules_tag_global);
  exec("init");
  //print_freelist(5);
  kprintf("returned from init\n");
  while (1);
  /*for (int i = 0; i < 5; i++) {
    p = (int*) pmem_alloc();
    kprintf("p: %p\n", p);
  }
  pmem_free((uintptr_t)p);
  for (int i = 0; i < 5; i++) {
    p = (int*) pmem_alloc();
    kprintf("p: %p\n", p);
  }

  // Loop forever, reading characters
  while (1) {
    kprintf("%c", kgetc());
  }*/

	// We're done, just hang...
	halt();
}
