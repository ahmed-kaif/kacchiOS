/* pic.c - Programmable Interrupt Controller implementation */
#include "pic.h"
#include "serial.h"

/* Initialize PIC */
void pic_init(void)
{
    /* Remap PIC using assembly function */
    pic_remap();

    serial_puts("PIC remapped (IRQ 0-15 -> INT 32-47).\n");
}
