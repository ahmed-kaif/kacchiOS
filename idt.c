/* idt.c - Interrupt Descriptor Table implementation */
#include "idt.h"
#include "io.h"
#include "serial.h"
#include "helper.h"

/* IDT with 256 entries */
struct idt_entry_struct idt_entries[256];
struct idt_ptr_struct idt_ptr;

/* External assembly function */
extern void idt_flush(uint32_t);

/* IRQ handler function pointers */
void *irq_routines[16] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

/* Exception messages */
char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

/* Remap the PIC (Programmable Interrupt Controller) */
static void pic_remap(void) {
    /* ICW1 - Initialize PIC */
    outb(0x20, 0x11);  /* Master PIC command port */
    outb(0xA0, 0x11);  /* Slave PIC command port */

    /* ICW2 - Set vector offsets */
    outb(0x21, 0x20);  /* Master PIC starts at IRQ 32 */
    outb(0xA1, 0x28);  /* Slave PIC starts at IRQ 40 */

    /* ICW3 - Setup cascading */
    outb(0x21, 0x04);  /* Master PIC has slave at IRQ2 */
    outb(0xA1, 0x02);  /* Slave PIC cascade identity */

    /* ICW4 - Set mode */
    outb(0x21, 0x01);  /* 8086 mode for master */
    outb(0xA1, 0x01);  /* 8086 mode for slave */

    /* Unmask all interrupts (OCW1) */
    outb(0x21, 0x0);   /* Enable all IRQs on master */
    outb(0xA1, 0x0);   /* Enable all IRQs on slave */
}

/* Initialize the IDT */
void idt_init(void) {
    idt_ptr.limit = sizeof(struct idt_entry_struct) * 256 - 1;
    idt_ptr.base = (uint32_t)&idt_entries;

    /* Clear the IDT */
    memset(&idt_entries, 0, sizeof(struct idt_entry_struct) * 256);

    /* Remap the PIC */
    pic_remap();

    /* Setup ISR handlers (CPU exceptions 0-31) */
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8E);
    idt_set_gate(2, (uint32_t)isr2, 0x08, 0x8E);
    idt_set_gate(3, (uint32_t)isr3, 0x08, 0x8E);
    idt_set_gate(4, (uint32_t)isr4, 0x08, 0x8E);
    idt_set_gate(5, (uint32_t)isr5, 0x08, 0x8E);
    idt_set_gate(6, (uint32_t)isr6, 0x08, 0x8E);
    idt_set_gate(7, (uint32_t)isr7, 0x08, 0x8E);
    idt_set_gate(8, (uint32_t)isr8, 0x08, 0x8E);
    idt_set_gate(9, (uint32_t)isr9, 0x08, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);

    /* Setup IRQ handlers (hardware interrupts 32-47) */
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2, 0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3, 0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4, 0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5, 0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6, 0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7, 0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8, 0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9, 0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);

    /* Setup system call handlers */
    idt_set_gate(128, (uint32_t)isr128, 0x08, 0x8E);
    idt_set_gate(177, (uint32_t)isr177, 0x08, 0x8E);

    /* Load the IDT */
    idt_flush((uint32_t)&idt_ptr);

    serial_puts("IDT initialized with 256 descriptors.\n");
    serial_puts("PIC remapped (IRQs 32-47).\n");
}

/* Set an IDT gate (descriptor) */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_low = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    idt_entries[num].sel = sel;
    idt_entries[num].always0 = 0;
    /* Set present bit (0x80) and privilege level to 3 (0x60) */
    idt_entries[num].flags = flags | 0x60;
}

/* ISR handler - handles CPU exceptions */
void isr_handler(struct interrupt_registers *regs) {
    if (regs->int_no < 32) {
        serial_puts("\n*** CPU Exception: ");
        serial_puts(exception_messages[regs->int_no]);
        serial_puts(" ***\n");
        serial_puts("System Halted!\n");
        
        /* Halt the system on exception */
        for (;;) {
            __asm__ volatile("cli; hlt");
        }
    }
}

/* Install an IRQ handler */
void irq_install_handler(int irq, void (*handler)(struct interrupt_registers *r)) {
    irq_routines[irq] = handler;
}

/* Uninstall an IRQ handler */
void irq_uninstall_handler(int irq) {
    irq_routines[irq] = 0;
}

/* IRQ handler - handles hardware interrupts */
void irq_handler(struct interrupt_registers *regs) {
    void (*handler)(struct interrupt_registers *regs);

    /* Call the registered handler if it exists */
    handler = irq_routines[regs->int_no - 32];
    if (handler) {
        handler(regs);
    }

    /* Send EOI (End of Interrupt) to PIC */
    if (regs->int_no >= 40) {
        /* Send EOI to slave PIC if interrupt came from IRQ8-15 */
        outb(0xA0, 0x20);
    }
    /* Always send EOI to master PIC */
    outb(0x20, 0x20);
}
