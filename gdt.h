/* gdt.h - Global Descriptor Table interface */
#ifndef GDT_H
#define GDT_H

#include "types.h"

/* GDT entry structure - describes a segment descriptor */
struct gdt_entry_struct {
    uint16_t limit;       /* Limit (bits 0-15) */
    uint16_t base_low;    /* Base (bits 0-15) */
    uint8_t base_middle;  /* Base (bits 16-23) */
    uint8_t access;       /* Access flags */
    uint8_t flags;        /* Granularity + Limit (bits 16-19) */
    uint8_t base_high;    /* Base (bits 24-31) */
} __attribute__((packed));

/* GDT pointer structure - passed to LGDT instruction */
struct gdt_ptr_struct {
    uint16_t limit;       /* Size of GDT - 1 */
    uint32_t base;        /* Base address of GDT */
} __attribute__((packed));

/* TSS (Task State Segment) structure for hardware multitasking */
struct tss_entry_struct {
    uint32_t prev_tss;    /* Previous TSS - not used */
    uint32_t esp0;        /* Stack pointer for ring 0 */
    uint32_t ss0;         /* Stack segment for ring 0 */
    uint32_t esp1;        /* Stack pointer for ring 1 */
    uint32_t ss1;         /* Stack segment for ring 1 */
    uint32_t esp2;        /* Stack pointer for ring 2 */
    uint32_t ss2;         /* Stack segment for ring 2 */
    uint32_t cr3;         /* Page directory base */
    uint32_t eip;         /* Instruction pointer */
    uint32_t eflags;      /* Flags register */
    uint32_t eax;         /* General purpose registers */
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;          /* Segment selectors */
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;         /* LDT selector */
    uint32_t trap;        /* Trap on task switch */
    uint32_t iomap_base;  /* I/O map base address */
} __attribute__((packed));

/* Initialize the GDT */
void gdt_init(void);

/* Set a GDT gate (descriptor) */
void gdt_set_gate(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

/* Write TSS entry */
void gdt_write_tss(uint32_t num, uint16_t ss0, uint32_t esp0);

#endif /* GDT_H */
