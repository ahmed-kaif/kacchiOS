/* pic.h - Programmable Interrupt Controller interface */
#ifndef PIC_H
#define PIC_H

#include "types.h"

/* External assembly function to remap PIC */
extern void pic_remap(void);

/* Enable/disable interrupts */
extern void interrupts_enable(void);
extern void interrupts_disable(void);

/* Initialize PIC */
void pic_init(void);

#endif /* PIC_H */
