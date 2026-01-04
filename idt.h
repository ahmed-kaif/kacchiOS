/* idt.h - Interrupt Descriptor Table interface */
#ifndef IDT_H
#define IDT_H

#include "types.h"

/* IDT entry structure - describes an interrupt/trap gate */
struct idt_entry_struct {
    uint16_t base_low;    /* Lower 16 bits of handler address */
    uint16_t sel;         /* Kernel segment selector */
    uint8_t always0;      /* Must be zero */
    uint8_t flags;        /* Type and attributes */
    uint16_t base_high;   /* Upper 16 bits of handler address */
} __attribute__((packed));

/* IDT pointer structure - passed to LIDT instruction */
struct idt_ptr_struct {
    uint16_t limit;       /* Size of IDT - 1 */
    uint32_t base;        /* Base address of IDT */
} __attribute__((packed));

/* Interrupt registers pushed on stack */
struct interrupt_registers {
    uint32_t cr2;         /* Page fault linear address */
    uint32_t ds;          /* Data segment selector */
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* Pushed by pusha */
    uint32_t int_no, err_code;  /* Interrupt number and error code */
    uint32_t eip, cs, eflags, useresp, ss;  /* Pushed by CPU */
};

/* Initialize the IDT */
void idt_init(void);

/* Set an IDT gate (descriptor) */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

/* Install an IRQ handler */
void irq_install_handler(int irq, void (*handler)(struct interrupt_registers *r));

/* Uninstall an IRQ handler */
void irq_uninstall_handler(int irq);

/* ISR (Interrupt Service Routine) handlers - CPU exceptions (0-31) */
extern void isr0(void);   /* Division by zero */
extern void isr1(void);   /* Debug */
extern void isr2(void);   /* Non-maskable interrupt */
extern void isr3(void);   /* Breakpoint */
extern void isr4(void);   /* Overflow */
extern void isr5(void);   /* Bound range exceeded */
extern void isr6(void);   /* Invalid opcode */
extern void isr7(void);   /* Device not available */
extern void isr8(void);   /* Double fault */
extern void isr9(void);   /* Coprocessor segment overrun */
extern void isr10(void);  /* Invalid TSS */
extern void isr11(void);  /* Segment not present */
extern void isr12(void);  /* Stack segment fault */
extern void isr13(void);  /* General protection fault */
extern void isr14(void);  /* Page fault */
extern void isr15(void);  /* Reserved */
extern void isr16(void);  /* x87 floating point exception */
extern void isr17(void);  /* Alignment check */
extern void isr18(void);  /* Machine check */
extern void isr19(void);  /* SIMD floating point exception */
extern void isr20(void);  /* Virtualization exception */
extern void isr21(void);  /* Reserved */
extern void isr22(void);  /* Reserved */
extern void isr23(void);  /* Reserved */
extern void isr24(void);  /* Reserved */
extern void isr25(void);  /* Reserved */
extern void isr26(void);  /* Reserved */
extern void isr27(void);  /* Reserved */
extern void isr28(void);  /* Reserved */
extern void isr29(void);  /* Reserved */
extern void isr30(void);  /* Reserved */
extern void isr31(void);  /* Reserved */

/* IRQ (Hardware Interrupt) handlers (32-47) */
extern void irq0(void);   /* PIT (Programmable Interval Timer) */
extern void irq1(void);   /* Keyboard */
extern void irq2(void);   /* Cascade (never raised) */
extern void irq3(void);   /* COM2 */
extern void irq4(void);   /* COM1 */
extern void irq5(void);   /* LPT2 */
extern void irq6(void);   /* Floppy disk */
extern void irq7(void);   /* LPT1 */
extern void irq8(void);   /* CMOS real-time clock */
extern void irq9(void);   /* Free for peripherals */
extern void irq10(void);  /* Free for peripherals */
extern void irq11(void);  /* Free for peripherals */
extern void irq12(void);  /* PS/2 mouse */
extern void irq13(void);  /* FPU / Coprocessor / Inter-processor */
extern void irq14(void);  /* Primary ATA hard disk */
extern void irq15(void);  /* Secondary ATA hard disk */

/* System call interrupts */
extern void isr128(void); /* System call (0x80) */
extern void isr177(void); /* Alternative system call (0xB1) */

#endif /* IDT_H */
