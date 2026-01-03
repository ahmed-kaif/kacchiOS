/* scheduler.h - Process scheduler interface */
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "types.h"
#include "process.h"

/* Time slice for round-robin scheduling (in timer ticks) */
#define TIME_SLICE_DEFAULT 10

/* Initialize the scheduler */
void scheduler_init(void);

/* Start the scheduler (begins multitasking) */
void scheduler_start(void);

/* Schedule next process (called by timer interrupt) */
void scheduler_schedule(void);

/* Yield CPU to next process */
void scheduler_yield(void);

/* Get scheduler statistics */
void scheduler_stats(void);

#endif
