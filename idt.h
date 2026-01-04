/* idt.h - Interrupt Descriptor Table interface */
#ifndef IDT_H
#define IDT_H

#include "types.h"

/* IDT entry structure */
typedef struct
{
    uint16_t base_low;  /* Lower 16 bits of handler address */
    uint16_t selector;  /* Kernel segment selector */
    uint8_t zero;       /* Always 0 */
    uint8_t flags;      /* Type and attributes */
    uint16_t base_high; /* Upper 16 bits of handler address */
} __attribute__((packed)) idt_entry_t;

/* IDT pointer structure for LIDT instruction */
typedef struct
{
    uint16_t limit; /* Size of IDT - 1 */
    uint32_t base;  /* Address of IDT */
} __attribute__((packed)) idt_ptr_t;

/* Number of IDT entries */
#define IDT_ENTRIES 256

/* Initialize IDT */
void idt_init(void);

/* Set an IDT entry */
void idt_set_gate(uint8_t num, uint32_t handler, uint16_t selector, uint8_t flags);

#endif /* IDT_H */
