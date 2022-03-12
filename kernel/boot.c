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
#include "strlib.h"
#include "elf.h"

#define MEMMAP_TAG_ID 0x2187f79e8612de07
#define HHDM_TAG_ID 0xb0ed257db18cb58f
#define MODULES_TAG_ID 0x4b6fe466aade04ce

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

// Probably add check that the requested module is an elf
void run_exec_elf(char* mod_name, struct stivale2_struct_tag_modules* modules_tag) {
  // Get the number of modules
  uint16_t count = modules_tag->module_count;
  struct stivale2_module* current;
  elf_hdr_t* elf_hdr = NULL;
  elf_phdr_t* elf_phdr = NULL;
  uint16_t phnum;
  int index = 0;
  // Loop over the modules until the requested one is found
  while (index < count) {
    current = &modules_tag->modules[index++];
    if (strcmp(current->string, mod_name) == 0) {
      // Cast the module start address to the elf header
      elf_hdr = (elf_hdr_t*) current->begin;
      // Calculate and store the address of the start of the program header table 
      elf_phdr = (elf_phdr_t*) (current->begin + elf_hdr->e_phoff);
      phnum = elf_hdr->e_phnum;
      break;
    } else if (index >= count) {
      kprintf("run_exec_elf: requested file not found in modules\n");
      return;
    }
  }
  // Make sure the file is executable
  if (elf_hdr->e_type != ET_EXEC) {
    kprintf("run_exec_elf: attempted to execute non-executable ELF file\n");
    return;
  }
  
  // Loop over the program table entries and allocate memory for loadable segments
  for (int i = 0; i < phnum; i++) {
    if (elf_phdr->p_type == PT_LOAD) {
      // Skip NULL sections
      if (elf_phdr->p_vaddr == 0x0) continue;
      
      // Allocate the segment's requested memory; if it is larger than a page, call vm_map enough times to accomodate the requested chunk
      uint64_t size_left = elf_phdr->p_memsz;
      int i = 0;
      do {
        // Allocate a requested page, setting permissions to writable only for byte copying
        if (vm_map(read_cr3(), elf_phdr->p_vaddr + (i * PAGE_SIZE), 0, 1, 1) == false) {
          kprintf("run_exec_elf: failed to allocate memory for requested page %p\n", elf_phdr->p_vaddr);
        }
        if (size_left >= PAGE_SIZE) size_left -= PAGE_SIZE;
        i++;
      } while (size_left >= PAGE_SIZE);

      // Copy the segment's data into the requested page
      memcpy((void*)elf_phdr->p_vaddr, (const void*) ((uintptr_t) elf_hdr + (uintptr_t) elf_phdr->p_offset), elf_phdr->p_filesz);
      
      // Change the permissions to those requested by the file.
      vm_protect(read_cr3(), elf_phdr->p_vaddr, ((elf_phdr->p_flags & 0x4) >> 2), ((elf_phdr->p_flags & 0x2) >> 1), ~(elf_phdr->p_flags & 0x1) & 0x1);
    }
    elf_phdr++;
  }
  // Cast the entry point to a function pointer and jump to it
  void (*entry_point) (void) = (void (*) (void)) elf_hdr->e_entry;
  (*entry_point)();
}

void _start(struct stivale2_struct* hdr) {
  // We've booted! Let's start processing tags passed to use from the bootloader
  term_setup(hdr);
  // Find the hhdm tag
  struct stivale2_struct_tag_hhdm* hhdm_tag = find_tag(hdr, HHDM_TAG_ID);
  hhdm_base_global = hhdm_tag->addr;
  // Find the module tag
  struct stivale2_struct_tag_modules* modules_tag = find_tag(hdr, MODULES_TAG_ID);

  // Initialize PIC
  pic_init();
  // Initialize interrupt descriptor table
  idt_setup();
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

  // Process modules
  run_exec_elf("init", modules_tag);

  // Loop forever, reading characters
  while (1) {
    kprintf("%c", kgetc());
  }

	// We're done, just hang...
	halt();
}