/* gdt.c - Global Descriptor Table implementation */
#include "gdt.h"
#include "serial.h"
#include "helper.h"

/* External assembly function */
extern void gdt_flush(uint32_t);
extern void tss_flush(void);

/* GDT with 6 entries: null, kernel code, kernel data, user code, user data, TSS */
struct gdt_entry_struct gdt_entries[6];
struct gdt_ptr_struct gdt_ptr;
struct tss_entry_struct tss_entry;

/* Initialize the GDT */
void gdt_init(void) {
    gdt_ptr.limit = (sizeof(struct gdt_entry_struct) * 6) - 1;
    gdt_ptr.base = (uint32_t)&gdt_entries;

    /* Setup GDT entries:
     * 0: Null descriptor (required by x86)
     * 1: Kernel code segment (0x08)
     * 2: Kernel data segment (0x10)
     * 3: User code segment (0x18)
     * 4: User data segment (0x20)
     * 5: TSS (Task State Segment) (0x28)
     */
    gdt_set_gate(0, 0, 0, 0, 0);                       /* Null descriptor */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);       /* Kernel code: base=0, limit=4GB, exec/read */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);       /* Kernel data: base=0, limit=4GB, read/write */
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);       /* User code: base=0, limit=4GB, exec/read */
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);       /* User data: base=0, limit=4GB, read/write */
    
    /* Initialize TSS with kernel stack segment (0x10) and no stack pointer (0x0) */
    gdt_write_tss(5, 0x10, 0x0);

    /* Load GDT and TSS */
    gdt_flush((uint32_t)&gdt_ptr);
    tss_flush();

    serial_puts("GDT initialized with 6 descriptors.\n");
}

/* Set a GDT gate (descriptor) */
void gdt_set_gate(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    /* Setup the descriptor base address */
    gdt_entries[num].base_low = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high = (base >> 24) & 0xFF;

    /* Setup the descriptor limits */
    gdt_entries[num].limit = (limit & 0xFFFF);
    gdt_entries[num].flags = (limit >> 16) & 0x0F;
    
    /* Granularity and flags */
    gdt_entries[num].flags |= (gran & 0xF0);
    
    /* Access flags */
    gdt_entries[num].access = access;
}

/* Write TSS entry */
void gdt_write_tss(uint32_t num, uint16_t ss0, uint32_t esp0) {
    uint32_t base = (uint32_t)&tss_entry;
    uint32_t limit = base + sizeof(tss_entry);

    /* Add TSS descriptor to GDT */
    gdt_set_gate(num, base, limit, 0xE9, 0x00);
    
    /* Clear the TSS */
    memset(&tss_entry, 0, sizeof(tss_entry));

    /* Set up kernel stack for interrupt handling */
    tss_entry.ss0 = ss0;
    tss_entry.esp0 = esp0;

    /* Set up segment registers (ring 3 with RPL=3) */
    tss_entry.cs = 0x08 | 0x3;  /* Kernel code segment | ring 3 */
    tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x10 | 0x3;
}
