/* scheduler.c - Process scheduler implementation */
#include "scheduler.h"
#include "process.h"
#include "serial.h"
#include "helper.h"

/* External assembly functions */
extern void switch_to_process(process_t *old_proc, process_t *new_proc);
extern void start_first_process(process_t *proc);

/* Scheduler state */
static uint32_t scheduler_enabled = 0;
static uint32_t total_context_switches = 0;
static uint32_t total_ticks = 0;
static uint32_t time_quantum = TIME_SLICE_DEFAULT; /* Configurable time quantum */

/* Find next ready process using round-robin */
static process_t *find_next_process(void)
{
    process_t *current = process_current();
    int start_idx = 0;
    int i;

    /* If there's a current process, start searching from next slot */
    if (current != NULL)
    {
        for (i = 0; i < MAX_PROCESSES; i++)
        {
            if (process_get(i) == current)
            {
                start_idx = (i + 1) % MAX_PROCESSES;
                break;
            }
        }
    }

    /* Search for next ready process (round-robin) */
    for (i = 0; i < MAX_PROCESSES; i++)
    {
        int idx = (start_idx + i) % MAX_PROCESSES;
        process_t *proc = process_get(idx);

        if (proc != NULL && proc->state == PROC_READY)
        {
            return proc;
        }
    }

    /* If current process is still running, keep it */
    if (current != NULL && current->state == PROC_RUNNING)
    {
        return current;
    }

    return NULL;
}

/* Initialize the scheduler */
void scheduler_init(void)
{
    scheduler_enabled = 0;
    total_context_switches = 0;
    total_ticks = 0;
    time_quantum = TIME_SLICE_DEFAULT;

    serial_puts("Scheduler initialized (Round-Robin).\n");
    serial_puts("Time quantum: ");
    char num_str[12];
    int_to_str(time_quantum, num_str);
    serial_puts(num_str);
    serial_puts(" ticks\n");
}

/* Start the scheduler */
void scheduler_start(void)
{
    scheduler_enabled = 1;
    serial_puts("Scheduler started.\n");

    /* Find first ready process */
    process_t *first_proc = find_next_process();

    if (first_proc != NULL)
    {
        first_proc->state = PROC_RUNNING;
        process_set_current(first_proc->pid);

        serial_puts("Starting process: ");
        serial_puts(first_proc->name);
        serial_puts(" (PID: ");
        char pid_str[12];
        int_to_str(first_proc->pid, pid_str);
        serial_puts(pid_str);
        serial_puts(")\n");

        serial_puts("Note: Timer interrupts not implemented - scheduler runs on manual ticks.\n");
    }
    else
    {
        serial_puts("WARNING: No ready processes to run!\n");
    }
}

/* Context switch helper using assembly */
static void context_switch(process_t *old_proc, process_t *new_proc)
{
    if (old_proc == new_proc)
    {
        return;
    }

    /* Update process states */
    if (old_proc != NULL && old_proc->state == PROC_RUNNING)
    {
        old_proc->state = PROC_READY;
    }

    new_proc->state = PROC_RUNNING;

    /* Update current process tracking */
    process_set_current(new_proc->pid);

    total_context_switches++;

    /* Perform actual context switch using assembly */
    switch_to_process(old_proc, new_proc);
}

/* Schedule next process */
void scheduler_schedule(void)
{
    if (!scheduler_enabled)
    {
        return;
    }

    total_ticks++;

    process_t *current = process_current();

    /* Decrement time slice of current process */
    if (current != NULL && current->state == PROC_RUNNING)
    {
        if (current->time_slice > 0)
        {
            current->time_slice--;
            current->total_time++;
        }

        /* If time slice expired, find next process */
        if (current->time_slice == 0)
        {
            current->time_slice = time_quantum;

            process_t *next = find_next_process();

            if (next != NULL && next != current)
            {
                serial_puts("[SCHED] Switching from ");
                serial_puts(current->name);
                serial_puts(" to ");
                serial_puts(next->name);
                serial_puts("\n");

                context_switch(current, next);
            }
        }
    }
    else
    {
        /* No current process or current is blocked, find next */
        process_t *next = find_next_process();

        if (next != NULL)
        {
            context_switch(current, next);
        }
    }
}

/* Yield CPU to next process */
void scheduler_yield(void)
{
    if (!scheduler_enabled)
    {
        return;
    }

    process_t *current = process_current();

    if (current != NULL)
    {
        /* Reset time slice and find next process */
        current->time_slice = time_quantum;

        process_t *next = find_next_process();

        if (next != NULL)
        {
            serial_puts("[YIELD] Process ");
            serial_puts(current->name);
            serial_puts(" yielding to ");
            serial_puts(next->name);
            serial_puts("\n");

            context_switch(current, next);
        }
    }
}

/* Get scheduler statistics */
void scheduler_stats(void)
{
    char num_str[12];

    serial_puts("\n=== Scheduler Statistics ===\n");

    serial_puts("Status: ");
    if (scheduler_enabled)
    {
        serial_puts("ENABLED\n");
    }
    else
    {
        serial_puts("DISABLED\n");
    }

    serial_puts("Total ticks: ");
    int_to_str(total_ticks, num_str);
    serial_puts(num_str);
    serial_puts("\n");

    serial_puts("Context switches: ");
    int_to_str(total_context_switches, num_str);
    serial_puts(num_str);
    serial_puts("\n");

    serial_puts("Time quantum: ");
    int_to_str(time_quantum, num_str);
    serial_puts(num_str);
    serial_puts(" ticks\n");

    process_t *current = process_current();
    if (current != NULL)
    {
        serial_puts("Current process: ");
        serial_puts(current->name);
        serial_puts(" (PID: ");
        int_to_str(current->pid, num_str);
        serial_puts(num_str);
        serial_puts(")\n");

        serial_puts("Remaining time slice: ");
        int_to_str(current->time_slice, num_str);
        serial_puts(num_str);
        serial_puts("\n");

        serial_puts("Total CPU time: ");
        int_to_str(current->total_time, num_str);
        serial_puts(num_str);
        serial_puts(" ticks\n");
    }
    else
    {
        serial_puts("Current process: NONE\n");
    }

    serial_puts("\n");
}

/* Set time quantum (configurable time slice) */
void scheduler_set_quantum(uint32_t quantum)
{
    if (quantum > 0)
    {
        time_quantum = quantum;
        serial_puts("Time quantum set to: ");
        char num_str[12];
        int_to_str(time_quantum, num_str);
        serial_puts(num_str);
        serial_puts(" ticks\n");
    }
    else
    {
        serial_puts("ERROR: Invalid time quantum (must be > 0)\n");
    }
}

/* Get current time quantum */
uint32_t scheduler_get_quantum(void)
{
    return time_quantum;
}
