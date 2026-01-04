/* timer.h - Programmable Interval Timer interface */
#ifndef TIMER_H
#define TIMER_H

#include "types.h"

/* PIT I/O ports */
#define PIT_CHANNEL0 0x40
#define PIT_CHANNEL1 0x41
#define PIT_CHANNEL2 0x42
#define PIT_COMMAND 0x43

/* PIT frequency in Hz */
#define PIT_FREQUENCY 1193180

/* Initialize the timer */
void timer_init(uint32_t frequency);

/* Get current tick count */
uint32_t timer_get_ticks(void);

#endif /* TIMER_H */
