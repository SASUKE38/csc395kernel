#include <stdint.h>
#include <stdbool.h>

#include "kprint.h"
#include "boot.h"

typedef struct page_table_entry {
  bool present : 1;
  bool writable : 1;
  bool user : 1;
  uint16_t unused : 9;
  uint64_t address : 51;
  bool no_execute : 1;
} __attribute__((packed)) pt_entry_t;

uintptr_t read_cr3() {
  uintptr_t value;
  __asm__("mov %%cr3, %0" : "=r" (value));
  return value;
}

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
  kprintf("translating %p\n",  address);
  kprintf("   page offset = %p\n",  indices[0]);
  kprintf("   index 1 = %p\n",  indices[1]);
  kprintf("   index 2 = %p\n",  indices[2]);
  kprintf("   index 3 = %p\n",  indices[3]);
  kprintf("   index 4 = %p\n",  indices[4]);

  for (int i = 4; i > 0; i--) {
    uint16_t index = indices[i];
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
      table = (pt_entry_t*)phys_to_vir((void*)table_phys);//(table_phys + hhdm_base_global);
    } else {
      kprintf("  not present\n");
      return;
    }
  }
  kprintf("%p maps to %p", address, (table_phys + (((uintptr_t) address) & 0x0000000000000FFF)));
}
