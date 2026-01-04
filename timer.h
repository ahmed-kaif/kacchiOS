/* timer.h - PIT (Programmable Interval Timer) interface */
#ifndef TIMER_H
#define TIMER_H

#include "types.h"

/* Timer frequency in Hz (how many times per second the timer fires) */
#define TIMER_FREQ 1000  /* 100 Hz = 10ms per tick */

/* Initialize the PIT timer */
void timer_init(void);

/* Get the number of timer ticks since boot */
uint32_t timer_get_ticks(void);

/* Sleep for a specified number of timer ticks */
void timer_wait(uint32_t ticks);

#endif /* TIMER_H */
