/* interrupt.S - Interrupt handlers and IDT loader */
.section .text
.code32

/* Load IDT - called from C with pointer to IDT descriptor */
.global idt_load
idt_load:
    mov 4(%esp), %eax    /* Get IDT pointer from stack */
    lidt (%eax)          /* Load IDT */
    ret

/* PIC remapping ports */
.equ PIC1_CMD,  0x20
.equ PIC1_DATA, 0x21
.equ PIC2_CMD,  0xA0
.equ PIC2_DATA, 0xA1

/* Remap PIC - called from C */
.global pic_remap
pic_remap:
    /* Save masks */
    in $PIC1_DATA, %al
    mov %al, %cl
    in $PIC2_DATA, %al
    mov %al, %ch
    
    /* Start initialization */
    mov $0x11, %al
    out %al, $PIC1_CMD
    out %al, $PIC2_CMD
    
    /* Set vector offsets */
    mov $0x20, %al       /* Master PIC offset = 32 */
    out %al, $PIC1_DATA
    mov $0x28, %al       /* Slave PIC offset = 40 */
    out %al, $PIC2_DATA
    
    /* Set up cascade */
    mov $0x04, %al
    out %al, $PIC1_DATA
    mov $0x02, %al
    out %al, $PIC2_DATA
    
    /* Set 8086 mode */
    mov $0x01, %al
    out %al, $PIC1_DATA
    out %al, $PIC2_DATA
    
    /* Restore masks */
    mov %cl, %al
    out %al, $PIC1_DATA
    mov %ch, %al
    out %al, $PIC2_DATA
    
    ret

/* Enable interrupts */
.global interrupts_enable
interrupts_enable:
    sti
    ret

/* Disable interrupts */
.global interrupts_disable
interrupts_disable:
    cli
    ret

/* Timer interrupt handler (IRQ 0 -> interrupt 32) */
.extern timer_interrupt_handler
.extern scheduler_need_reschedule
.extern scheduler_switch_from_interrupt
.global irq0_handler
irq0_handler:
    /* Save all registers */
    pusha
    push %ds
    push %es
    push %fs
    push %gs
    
    /* Load kernel data segment */
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    
    /* Call C handler to increment ticks */
    call timer_interrupt_handler
    
    /* Check if we need to reschedule */
    call scheduler_need_reschedule
    test %eax, %eax
    jz no_reschedule
    
    /* Do context switch from interrupt */
    call scheduler_switch_from_interrupt
    
no_reschedule:
    /* Send EOI to PIC */
    mov $0x20, %al
    out %al, $0x20
    
    /* Restore registers */
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    
    /* Return from interrupt */
    iret
