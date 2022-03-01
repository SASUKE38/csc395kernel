#pragma once

#define IDT_TYPE_INTERRUPT 0xE
#define IDT_TYPE_TRAP 0xF

// Initializes an interrupt descriptor table.
void idt_setup();

/**
 * Set an interrupt handler for the given interrupt number.
 *
 * \param index The interrupt number to handle
 * \param fn    A pointer to the interrupt handler function
 * \param type  The type of interrupt handler being installed.
 *              Pass IDT_TYPE_INTERRUPT or IDT_TYPE_TRAP from above.
 */
void idt_set_handler(uint8_t index, void* fn, uint8_t type);
