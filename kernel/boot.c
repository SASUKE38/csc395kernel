#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "stivale2.h"
#include "util.h"

#include "kprint.h"
#include "idt.h"
#include "pic.h"
#include "key.h"
#include "page.h"
#include "syscall_def.h"

#define MAX_MEM_SECTIONS 10

#define SYS_read 0
#define SYS_write 1

uint64_t hhdm_base_global;

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

void term_setup(struct stivale2_struct* hdr) {
  // Look for a terminal tag
  struct stivale2_struct_tag_terminal* tag = find_tag(hdr, STIVALE2_STRUCT_TAG_TERMINAL_ID);

  // Make sure we find a terminal tag
  if (tag == NULL) halt();

  // Save the term_write function pointer
	set_term_write((term_write_t)tag->term_write);
}

// Print usable memory using memmap and hhdm tags
void print_mem_address(struct stivale2_struct* hdr) {
  // Find the hhdm and memmap tags in the list
  struct stivale2_struct_tag_hhdm* hhdm_tag = find_tag(hdr, 0xb0ed257db18cb58f);
  struct stivale2_struct_tag_memmap* memmap_tag = find_tag(hdr, 0x2187f79e8612de07);
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
  struct stivale2_struct_tag_memmap* memmap_tag = find_tag(hdr, 0x2187f79e8612de07);
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
  term_setup(hdr);
  struct stivale2_struct_tag_hhdm* hhdm_tag = find_tag(hdr, 0xb0ed257db18cb58f);
  hhdm_base_global = hhdm_tag->addr;
  // Initialize PIC
  pic_init();
  // Initialize interrupt descriptor table
  idt_setup();
  pic_unmask_irq(1);
  // Set handler for system calls
  idt_set_handler(0x80, syscall_entry, IDT_TYPE_TRAP);

  // Print usable memory ranges
  print_mem_address(hdr);

  //uintptr_t cr3_ptr = read_cr3();
  //translate(_start);

  // Enable write protection
  uint64_t cr0 = read_cr0();
  cr0 |= 0x10000;
  write_cr0(cr0);

  char buf[6];
  long rc = syscall(SYS_read, 0, buf, 5);
  if (rc <= 0) {
    kprintf("read failed\n");
  } else {
    buf[rc] = '\0';
    kprintf("read '%s'\n", buf);
  }

  kprintf("write test:\n");
  long rc2 = syscall(SYS_write, 1, "buf", 3);
  kprintf("\n");
  kprintf("rc2: %d\n", rc2);

  uint64_t get_addr_result[MAX_MEM_SECTIONS];
  uint64_t start[MAX_MEM_SECTIONS];
  uint64_t end[MAX_MEM_SECTIONS];
  uint16_t num_read;

  // Fill array to pass to freelist_init
  num_read = get_mem_address(hdr, get_addr_result);
  start[0] = get_addr_result[0];
  start[1] = get_addr_result[2];
  end[0] = get_addr_result[1];
  end[1] = get_addr_result[3];
  freelist_init(start, end, 1);
  print_freelist(5);
  kprintf("\n");

  // pmem_alloc() tests
  /*uintptr_t test_ptr1 = pmem_alloc();
  uintptr_t test_ptr2 = pmem_alloc();
  uintptr_t test_ptr3 = pmem_alloc();
  uintptr_t test_ptr4 = pmem_alloc();
  uintptr_t test_ptr5 = pmem_alloc();
  kprintf("Test 1: %p\n", test_ptr1);
  kprintf("Test 2: %p\n", test_ptr2);
  kprintf("Test 3: %p\n", test_ptr3);
  kprintf("Test 4: %p\n", test_ptr4);
  kprintf("Test 5: %p\n", test_ptr5);
  pmem_free(test_ptr1);
  uintptr_t test_ptr6 = pmem_alloc();
  kprintf("Test 6: %p\n", test_ptr6);
  pmem_free(test_ptr2);
  uintptr_t test_ptr7 = pmem_alloc();
  kprintf("Test 7: %p\n", test_ptr7);
  uintptr_t test_ptr8 = pmem_alloc();
  kprintf("Test 8: %p\n", test_ptr8);*/

  /*for (int i = 0; i < 4; i++) {
    kprintf("iteration %d\n", i);
    uintptr_t new_ptr = pmem_alloc();
    kprintf("pointer obtained: %p\n", new_ptr);
  }*/

  /*char* test_string = "aaaaaaaaaa";
  int num_read = 0;*/

  //uintptr_t root = read_cr3() & 0xFFFFFFFFFFFFF000;
  //int* p = (int*)0x5000400000;
  //bool result = vm_map(root, (uintptr_t)p, false, true, false);
  /*if (result) {
    *p = 123;
    kprintf("Stored %d at %p\n", *p, p);
  } else {
    kprintf("vm_map failed with an error\n");
  }*/

  //translate(p);

  /*result = vm_map(root, (uintptr_t)p, false, true, false);
  if (result) {
    kprintf("no error\n");
  } else {
    kprintf("didn't overwrite %d\n", *p);
  }

  result = vm_unmap(root, (uintptr_t)p);
  if (result == true) {
    kprintf("unmapped %p\n", p);
  } else {
    kprintf("unmap fauled\n");
  }

  *p = 1234;
  kprintf("%d", *p);*/

  while (1) {
    kprintf("%c", kgetc());
  }

  /*while (1) {
    kprintf("Line: \n");
    num_read = kgets(test_string, 10);
    kprintf("%s, %d",test_string, num_read);
    kprintf("\n");
  }*/

	// We're done, just hang...
	halt();
}
