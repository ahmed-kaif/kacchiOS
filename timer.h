#ifndef TIMER_H
#define TIMER_H

#include "types.h"

/* Timer frequency in Hz */
#define TIMER_FREQ 10000  /* 100 Hz = 10ms per tick */

/* Initialize the PIT timer */
void timer_init(void);

/* Get the number of timer ticks since boot */
uint32_t timer_get_ticks(void);

/* Sleep for a specified number of timer ticks */
void timer_wait(uint32_t ticks);

#endif /* TIMER_H */
