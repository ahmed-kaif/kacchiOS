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

/* Timer tick handler - called by timer interrupt */
void scheduler_tick(void);

/* Yield CPU to next process */
void scheduler_yield(void);

/* Get scheduler statistics */
void scheduler_stats(void);

/* Set time quantum (configurable time slice) */
void scheduler_set_quantum(uint32_t quantum);

/* Get current time quantum */
uint32_t scheduler_get_quantum(void);

#endif
