/* gdt.S - GDT assembly functions */
.section .text

/* Load GDT and update segment registers */
.global gdt_flush
.type gdt_flush, @function
gdt_flush:
    mov 4(%esp), %eax    /* Get GDT pointer parameter */
    lgdt (%eax)          /* Load GDT */

    /* Update segment registers with kernel data segment (0x10) */
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    
    /* Far jump to update CS register with kernel code segment (0x08) */
    jmp $0x08, $.flush
.flush:
    ret

/* Load TSS (Task State Segment) */
.global tss_flush
.type tss_flush, @function
tss_flush:
    mov $0x2B, %ax       /* TSS segment selector (5 * 8 + 3) */
    ltr %ax              /* Load task register */
    ret
