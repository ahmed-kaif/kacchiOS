/* idt.c - Interrupt Descriptor Table implementation */
#include "idt.h"
#include "serial.h"

/* IDT entries */
static idt_entry_t idt[IDT_ENTRIES];

/* IDT pointer */
static idt_ptr_t idt_ptr;

/* External assembly function to load IDT */
extern void idt_load(uint32_t idt_ptr_addr);

/* Set an IDT entry */
void idt_set_gate(uint8_t num, uint32_t handler, uint16_t selector, uint8_t flags)
{
    idt[num].base_low = handler & 0xFFFF;
    idt[num].base_high = (handler >> 16) & 0xFFFF;
    idt[num].selector = selector;
    idt[num].zero = 0;
    idt[num].flags = flags;
}

/* Initialize IDT */
void idt_init(void)
{
    int i;

    /* Set up IDT pointer */
    idt_ptr.limit = (sizeof(idt_entry_t) * IDT_ENTRIES) - 1;
    idt_ptr.base = (uint32_t)&idt;

    /* Clear all IDT entries */
    for (i = 0; i < IDT_ENTRIES; i++)
    {
        idt[i].base_low = 0;
        idt[i].base_high = 0;
        idt[i].selector = 0;
        idt[i].zero = 0;
        idt[i].flags = 0;
    }

    /* Load the IDT */
    idt_load((uint32_t)&idt_ptr);

    serial_puts("IDT initialized.\n");
}
