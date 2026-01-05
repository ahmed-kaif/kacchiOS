/* scheduler.c - Process scheduler implementation */
#include "scheduler.h"
#include "process.h"
#include "serial.h"
#include "helper.h"
#include "types.h"

/* External assembly functions */
extern void switch_to_process(process_t *old_proc, process_t *new_proc);
extern void start_first_process(process_t *proc);

/* Scheduler state */
static uint32_t scheduler_enabled = 0;
uint32_t total_context_switches = 0;  /* Made non-static for timer.c */
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
    serial_puts("[SCHED] Starting scheduler...\n");
    scheduler_enabled = 1;
    serial_puts("Scheduler started.\n");

    /* Find first ready process */
    process_t *first_proc = find_next_process();
    
    serial_puts("[SCHED] Finding first process...\n");

    if (first_proc != NULL)
    {
        serial_puts("[SCHED] Found process: ");
        serial_puts(first_proc->name);
        serial_puts("\n");
        
        first_proc->state = PROC_RUNNING;
        process_set_current(first_proc->pid);

        serial_puts("Starting process: ");
        serial_puts(first_proc->name);
        serial_puts(" (PID: ");
        char pid_str[12];
        int_to_str(first_proc->pid, pid_str);
        serial_puts(pid_str);
        serial_puts(")\n");
        serial_puts("Timer-based preemptive multitasking enabled (100 Hz).\n");

        serial_puts("[SCHED] About to call start_first_process...\n");
        
        /* Enable interrupts before starting the first process */
        __asm__ volatile("sti");
        
        /* Get the entry point from the process stack */
        void (*entry_point)(void) = (void (*)(void)) *((uint32_t *)first_proc->stack_ptr + 7);
        
        serial_puts("[SCHED] Jumping to first process directly...\n");
        
        /* Just call it directly for now */
        entry_point();
        
        serial_puts("[SCHED] ERROR: Returned from first process!\n");
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

    serial_puts("[CTX] About to call switch_to_process\n");
    
    /* Perform actual context switch using assembly */
    switch_to_process(old_proc, new_proc);
    
    serial_puts("[CTX] Returned from switch_to_process - THIS SHOULD APPEAR!\n");
    
    /* CRITICAL FIX: Re-enable interrupts after context switch!
     * The context switch may leave interrupts disabled, so we explicitly enable them */
    __asm__ volatile("sti");
}

/* Schedule next process */
void scheduler_schedule(void)
{
    static uint32_t schedule_calls = 0;
    schedule_calls++;
    
    if (schedule_calls <= 30 || schedule_calls % 1000 == 0) {
        serial_puts("[SCHED_SCHEDULE] Call ");
        char num_str[12];
        int_to_str(schedule_calls, num_str);
        serial_puts(num_str);
        serial_puts(", enabled=");
        int_to_str(scheduler_enabled, num_str);
        serial_puts(num_str);
        serial_puts("\n");
    }
    
    if (!scheduler_enabled)
    {
        return;
    }

    total_ticks++;

    process_t *current = process_current();
    
    if (schedule_calls <= 30) {
        serial_puts("[DEBUG] current=");
        if (current) {
            serial_puts(current->name);
            serial_puts(", state=");
            char num_str[12];
            int_to_str(current->state, num_str);
            serial_puts(num_str);
            serial_puts(", time_slice=");
            int_to_str(current->time_slice, num_str);
            serial_puts(num_str);
        } else {
            serial_puts("NULL");
        }
        serial_puts("\n");
    }

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
            serial_puts("[SCHED] Time slice expired for ");
            serial_puts(current->name);
            serial_puts(", time_slice reset\n");
            
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
                
                serial_puts("[SCHED] Returned from context_switch\n");
            }
            else {
                serial_puts("[SCHED] No other process to switch to\n");
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

/* Timer tick handler - called by timer interrupt */
void scheduler_tick(void)
{
    static uint32_t tick_calls = 0;
    tick_calls++;
    
    if (tick_calls % 1000 == 0) {
        serial_puts("[SCHED_TICK] Called ");
        char num_str[12];
        int_to_str(tick_calls, num_str);
        serial_puts(num_str);
        serial_puts(" times, enabled=");
        int_to_str(scheduler_enabled, num_str);
        serial_puts(num_str);
        serial_puts("\n");
    }
    
    /* Only schedule if scheduler is enabled */
    if (!scheduler_enabled)
    {
        return;
    }

    /* Call the scheduler to handle time slice and context switching */
    scheduler_schedule();
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
