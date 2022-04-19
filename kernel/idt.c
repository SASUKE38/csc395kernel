#include <stdint.h>

#include "util.h"
#include "kprint.h"
#include "strlib.h"
#include "pic.h"
#include "port.h"
#include "key.h"
#include "idt.h"
#include "gdt.h"

// This struct matches the layout of an interrupt context.
typedef struct interrupt_context {
  uintptr_t ip;
  uint64_t cs;
  uint64_t flags;
  uintptr_t sp;
  uint64_t ss;
} __attribute__((packed)) interrupt_context_t;

// Definitions of interrupt handlers
__attribute__((interrupt))
void divide_error_handler(interrupt_context_t* ctx) {
  kprintf("Fault: Divide by zero\n");
  halt();
}

__attribute__((interrupt))
void debug_exception_handler(interrupt_context_t* ctx) {
  kprintf("Fault: Debug exception\n");
  halt();
}

__attribute__((interrupt))
void NMI_interrupt_handler(interrupt_context_t* ctx) {
  kprintf("Interrupt: NMI interrupt\n");
  halt();
}

__attribute__((interrupt))
void breakpoint_handler(interrupt_context_t* ctx) {
  kprintf("Trap: Breakpoint\n");
  halt();
}

__attribute__((interrupt))
void overflow_handler(interrupt_context_t* ctx) {
  kprintf("Trap: Overflow\n");
  halt();
}

__attribute__((interrupt))
void bound_range_handler(interrupt_context_t* ctx) {
  kprintf("Fault: Bound range exceeded\n");
  halt();
}

__attribute__((interrupt))
void invalid_opcode_handler(interrupt_context_t* ctx) {
  kprintf("Fault: Invalid opcode\n");
  halt();
}

__attribute__((interrupt))
void device_not_available_handler(interrupt_context_t* ctx) {
  kprintf("Fault: Device not available\n");
  halt();
}

__attribute__((interrupt))
void double_fault_handler(interrupt_context_t* ctx, uint64_t ec) {
  kprintf("Abort: Double fault (ec=%d)\n", ec);
  halt();
}

__attribute__((interrupt))
void coprocessor_segment_overrun_handler(interrupt_context_t* ctx) {
  kprintf("Fault: Coprocessor segment overrun\n");
  halt();
}

__attribute__((interrupt))
void invalid_tss_handler(interrupt_context_t* ctx, uint64_t ec) {
  kprintf("Fault: Invalid tss (ec=%d)\n", ec);
  halt();
}

__attribute__((interrupt))
void segment_not_present_handler(interrupt_context_t* ctx, uint64_t ec) {
  kprintf("Fault: Segment not present (ec=%d)\n", ec);
  halt();
}

__attribute__((interrupt))
void stack_segment_fault_handler(interrupt_context_t* ctx, uint64_t ec) {
  kprintf("Fault: Stack-segment fault (ec=%d)\n", ec);
  halt();
}

__attribute__((interrupt))
void general_protection_handler(interrupt_context_t* ctx, uint64_t ec) {
  kprintf("Fault: General protection (ec=%d)\n", ec);
  halt();
}

__attribute__((interrupt))
void page_fault_handler(interrupt_context_t* ctx, uint64_t ec) {
  kprintf("Fault: Page fault (ec=%d)\n", ec);
  halt();
}

__attribute__((interrupt))
void floating_point_handler(interrupt_context_t* ctx) {
  kprintf("Fault: x87 FPU floating-point error\n");
  halt();
}

__attribute__((interrupt))
void alignment_check_handler(interrupt_context_t* ctx, uint64_t ec) {
  kprintf("Fault: alignment check (ec=%d)\n", ec);
  halt();
}

__attribute__((interrupt))
void machine_check_handler(interrupt_context_t* ctx) {
  kprintf("Abort: Machine check\n");
  halt();
}

__attribute__((interrupt))
void simd_floating_point_exception_handler(interrupt_context_t* ctx) {
  kprintf("Fault: SIMD floating-point exception\n");
  halt();
}

__attribute__((interrupt))
void virtualization_exception_handler(interrupt_context_t* ctx) {
  kprintf("Fault: Virtualization exception\n");
  halt();
}

__attribute__((interrupt))
void control_protection_exception_handler(interrupt_context_t* ctx, uint64_t ec) {
  kprintf("Fault: Control protection exception (ec=%d)\n", ec);
  halt();
}

__attribute__((interrupt))
void irq1_interrupt_handler(interrupt_context_t* ctx) {
  handle_press(inb(0x60));
  //kprintf("%p\n", inb(0x60));
  outb(PIC1_COMMAND, PIC_EOI);
}

// Every interrupt handler must specify a code selector. We'll use entry 5 (5*8=0x28), which
// is where our bootloader set up a usable code selector for 64-bit mode.
#define IDT_CODE_SELECTOR 0x28

// IDT entry types
#define IDT_TYPE_INTERRUPT 0xE
#define IDT_TYPE_TRAP 0xF

// A struct the matches the layout of an IDT entry
typedef struct idt_entry {
  uint16_t offset_0;
  uint16_t selector;
  uint8_t ist : 3;
  uint8_t _unused_0 : 5;
  uint8_t type : 4;
  uint8_t _unused_1 : 1;
  uint8_t dpl : 2;
  uint8_t present : 1;
  uint16_t offset_1;
  uint32_t offset_2;
  uint32_t _unused_2;
} __attribute__((packed)) idt_entry_t;

// Make an IDT
idt_entry_t idt[256];

/**
 * Set an interrupt handler for the given interrupt number.
 *
 * \param index The interrupt number to handle
 * \param fn    A pointer to the interrupt handler function
 * \param type  The type of interrupt handler being installed.
 *              Pass IDT_TYPE_INTERRUPT or IDT_TYPE_TRAP from above.
 */
void idt_set_handler(uint8_t index, void* fn, uint8_t type) {
  // Split the function pointer into pieces via bit masks and shifting

  idt[index].offset_0 = (uint16_t) ((uint64_t) fn) & 0x000000000000FFFF;
  idt[index].offset_1 = (uint16_t) ((((uint64_t) fn) & 0x00000000FFFF0000) >> 16);
  idt[index].offset_2 = (uint32_t) ((((uint64_t) fn) & 0xFFFFFFFF00000000) >> 32);

  idt[index].type = type;
  idt[index].present = 1; // entry is present
  idt[index].dpl = 3; // handler can be run in user mode
  idt[index].ist = 0; // we aren't using an interrupt stack table, so just pass 0
  idt[index].selector = KERNEL_CODE_SELECTOR;
}

// This struct is used to load an IDT once we've set it up
typedef struct idt_record {
  uint16_t size;
  void* base;
} __attribute__((packed)) idt_record_t;

/**
 * Initialize an interrupt descriptor table, set handlers for standard exceptions, and install
 * the IDT.
 */
void idt_setup() {
  // Zero out the IDT
  memset(idt, 0, 256);

  // Set handlers for the standard exceptions (0--21)
  idt_set_handler(0, &divide_error_handler, IDT_TYPE_TRAP);
  idt_set_handler(1, &debug_exception_handler, IDT_TYPE_TRAP);
  idt_set_handler(2, &NMI_interrupt_handler, IDT_TYPE_INTERRUPT);
  idt_set_handler(3, &breakpoint_handler, IDT_TYPE_TRAP);
  idt_set_handler(4, &overflow_handler, IDT_TYPE_TRAP);
  idt_set_handler(5, &bound_range_handler, IDT_TYPE_TRAP);
  idt_set_handler(6, &invalid_opcode_handler, IDT_TYPE_TRAP);
  idt_set_handler(7, &device_not_available_handler, IDT_TYPE_TRAP);
  idt_set_handler(8, &double_fault_handler, IDT_TYPE_TRAP);
  idt_set_handler(9, &coprocessor_segment_overrun_handler, IDT_TYPE_TRAP);
  idt_set_handler(10, &invalid_tss_handler, IDT_TYPE_TRAP);
  idt_set_handler(11, &segment_not_present_handler, IDT_TYPE_TRAP);
  idt_set_handler(12, &stack_segment_fault_handler, IDT_TYPE_TRAP);
  idt_set_handler(13, &general_protection_handler, IDT_TYPE_TRAP);
  idt_set_handler(14, &page_fault_handler, IDT_TYPE_TRAP);
  idt_set_handler(16, &floating_point_handler, IDT_TYPE_TRAP);
  idt_set_handler(17, &alignment_check_handler, IDT_TYPE_TRAP);
  idt_set_handler(18, &machine_check_handler, IDT_TYPE_TRAP);
  idt_set_handler(19, &simd_floating_point_exception_handler, IDT_TYPE_TRAP);
  idt_set_handler(20, &virtualization_exception_handler, IDT_TYPE_TRAP);
  idt_set_handler(21, &control_protection_exception_handler, IDT_TYPE_TRAP);
  idt_set_handler(IRQ1_INTERRUPT, &irq1_interrupt_handler, IDT_TYPE_INTERRUPT);

  // Install the IDT
  idt_record_t record = {
    .size = sizeof(idt),
    .base = idt
  };
  __asm__("lidt %0" :: "m"(record));
}
