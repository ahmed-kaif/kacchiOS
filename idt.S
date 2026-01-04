/* idt.S - Interrupt Descriptor Table assembly stubs */
.section .text

/* Load IDT and enable interrupts */
.global idt_flush
.type idt_flush, @function
idt_flush:
    mov 4(%esp), %eax    /* Get IDT pointer parameter */
    lidt (%eax)          /* Load IDT */
    sti                  /* Enable interrupts */
    ret

/* Macro to create ISR stub without error code */
.macro ISR_NOERRCODE num
    .global isr\num
    .type isr\num, @function
    isr\num:
        cli                 /* Disable interrupts */
        push $0             /* Push dummy error code */
        push $\num          /* Push interrupt number */
        jmp isr_common_stub
.endm

/* Macro to create ISR stub with error code */
.macro ISR_ERRCODE num
    .global isr\num
    .type isr\num, @function
    isr\num:
        cli                 /* Disable interrupts */
        push $\num          /* Push interrupt number (error code already pushed) */
        jmp isr_common_stub
.endm

/* Macro to create IRQ stub */
.macro IRQ irq_num int_num
    .global irq\irq_num
    .type irq\irq_num, @function
    irq\irq_num:
        cli                 /* Disable interrupts */
        push $0             /* Push dummy error code */
        push $\int_num      /* Push interrupt number */
        jmp irq_common_stub
.endm

/* CPU Exception ISRs (0-31) */
ISR_NOERRCODE 0         /* Division by zero */
ISR_NOERRCODE 1         /* Debug */
ISR_NOERRCODE 2         /* Non-maskable interrupt */
ISR_NOERRCODE 3         /* Breakpoint */
ISR_NOERRCODE 4         /* Overflow */
ISR_NOERRCODE 5         /* Bound range exceeded */
ISR_NOERRCODE 6         /* Invalid opcode */
ISR_NOERRCODE 7         /* Device not available */
ISR_ERRCODE 8           /* Double fault (has error code) */
ISR_NOERRCODE 9         /* Coprocessor segment overrun */
ISR_ERRCODE 10          /* Invalid TSS (has error code) */
ISR_ERRCODE 11          /* Segment not present (has error code) */
ISR_ERRCODE 12          /* Stack segment fault (has error code) */
ISR_ERRCODE 13          /* General protection fault (has error code) */
ISR_ERRCODE 14          /* Page fault (has error code) */
ISR_NOERRCODE 15        /* Reserved */
ISR_NOERRCODE 16        /* x87 floating point exception */
ISR_NOERRCODE 17        /* Alignment check */
ISR_NOERRCODE 18        /* Machine check */
ISR_NOERRCODE 19        /* SIMD floating point exception */
ISR_NOERRCODE 20        /* Virtualization exception */
ISR_NOERRCODE 21        /* Reserved */
ISR_NOERRCODE 22        /* Reserved */
ISR_NOERRCODE 23        /* Reserved */
ISR_NOERRCODE 24        /* Reserved */
ISR_NOERRCODE 25        /* Reserved */
ISR_NOERRCODE 26        /* Reserved */
ISR_NOERRCODE 27        /* Reserved */
ISR_NOERRCODE 28        /* Reserved */
ISR_NOERRCODE 29        /* Reserved */
ISR_NOERRCODE 30        /* Reserved */
ISR_NOERRCODE 31        /* Reserved */

/* System call ISRs */
ISR_NOERRCODE 128       /* System call (0x80) */
ISR_NOERRCODE 177       /* Alternative system call (0xB1) */

/* Hardware IRQs (32-47) */
IRQ 0, 32               /* PIT timer */
IRQ 1, 33               /* Keyboard */
IRQ 2, 34               /* Cascade */
IRQ 3, 35               /* COM2 */
IRQ 4, 36               /* COM1 */
IRQ 5, 37               /* LPT2 */
IRQ 6, 38               /* Floppy */
IRQ 7, 39               /* LPT1 */
IRQ 8, 40               /* CMOS RTC */
IRQ 9, 41               /* Free */
IRQ 10, 42              /* Free */
IRQ 11, 43              /* Free */
IRQ 12, 44              /* PS/2 Mouse */
IRQ 13, 45              /* FPU */
IRQ 14, 46              /* Primary ATA */
IRQ 15, 47              /* Secondary ATA */

/* Common ISR stub - handles all CPU exceptions */
.extern isr_handler
.type isr_common_stub, @function
isr_common_stub:
    pusha               /* Push all general purpose registers */
    mov %ds, %eax       /* Save data segment */
    push %eax
    mov %cr2, %eax      /* Save CR2 (page fault address) */
    push %eax

    /* Load kernel data segment */
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    /* Call C handler */
    push %esp           /* Pass pointer to stack frame */
    call isr_handler

    /* Restore registers */
    add $8, %esp        /* Remove CR2 and pushed ESP */
    pop %ebx            /* Restore data segment */
    mov %bx, %ds
    mov %bx, %es
    mov %bx, %fs
    mov %bx, %gs

    popa                /* Restore general purpose registers */
    add $8, %esp        /* Remove error code and interrupt number */
    sti                 /* Re-enable interrupts */
    iret                /* Return from interrupt */

/* Common IRQ stub - handles all hardware interrupts */
.extern irq_handler
.type irq_common_stub, @function
irq_common_stub:
    pusha               /* Push all general purpose registers */
    mov %ds, %eax       /* Save data segment */
    push %eax
    mov %cr2, %eax      /* Save CR2 (page fault address) */
    push %eax

    /* Load kernel data segment */
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    /* Call C handler */
    push %esp           /* Pass pointer to stack frame */
    call irq_handler

    /* Restore registers */
    add $8, %esp        /* Remove CR2 and pushed ESP */
    pop %ebx            /* Restore data segment */
    mov %bx, %ds
    mov %bx, %es
    mov %bx, %fs
    mov %bx, %gs

    popa                /* Restore general purpose registers */
    add $8, %esp        /* Remove error code and interrupt number */
    sti                 /* Re-enable interrupts */
    iret                /* Return from interrupt */
