#include <stdint.h>
#include <stddef.h>

#include "stivale2.h"
#include "util.h"

#include "kprint.h"
#include "idt.h"
#include "pic.h"

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

void _start(struct stivale2_struct* hdr) {
  // We've booted! Let's start processing tags passed to use from the bootloader
  term_setup(hdr);
  // Initialize interrupt descriptor table
  idt_setup();
  pic_init();
  // Print usable memory ranges
  print_mem_address(hdr);

	// We're done, just hang...
	halt();
}
