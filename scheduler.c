/* scheduler.c - Process scheduler implementation */
#include "scheduler.h"
#include "process.h"
#include "serial.h"

/* Scheduler state */
static uint32_t scheduler_enabled = 0;
static uint32_t total_context_switches = 0;
static uint32_t total_ticks = 0;

/* Helper function to convert integer to string */
static void int_to_str(int num, char *str) {
    int i = 0;
    int is_negative = 0;
    
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }
    
    while (num > 0) {
        str[i++] = (num % 10) + '0';
        num /= 10;
    }
    
    if (is_negative) {
        str[i++] = '-';
    }
    
    str[i] = '\0';
    
    /* Reverse the string */
    int j;
    for (j = 0; j < i / 2; j++) {
        char temp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = temp;
    }
}

/* Find next ready process using round-robin */
static process_t *find_next_process(void) {
    process_t *current = process_current();
    int start_idx = 0;
    int i;
    
    /* If there's a current process, start searching from next slot */
    if (current != NULL) {
        for (i = 0; i < MAX_PROCESSES; i++) {
            if (process_get(i) == current) {
                start_idx = (i + 1) % MAX_PROCESSES;
                break;
            }
        }
    }
    
    /* Search for next ready process (round-robin) */
    for (i = 0; i < MAX_PROCESSES; i++) {
        int idx = (start_idx + i) % MAX_PROCESSES;
        process_t *proc = process_get(idx);
        
        if (proc != NULL && proc->state == PROC_READY) {
            return proc;
        }
    }
    
    /* If current process is still running, keep it */
    if (current != NULL && current->state == PROC_RUNNING) {
        return current;
    }
    
    return NULL;
}

/* Initialize the scheduler */
void scheduler_init(void) {
    scheduler_enabled = 0;
    total_context_switches = 0;
    total_ticks = 0;
    
    serial_puts("Scheduler initialized (Round-Robin).\n");
}

/* Start the scheduler */
void scheduler_start(void) {
    scheduler_enabled = 1;
    serial_puts("Scheduler started.\n");
    
    /* Find first ready process */
    process_t *first_proc = find_next_process();
    
    if (first_proc != NULL) {
        first_proc->state = PROC_RUNNING;
        serial_puts("Starting process: ");
        serial_puts(first_proc->name);
        serial_puts("\n");
        
        /* TODO: Jump to first process when context switching is implemented */
    } else {
        serial_puts("WARNING: No ready processes to run!\n");
    }
}

/* Context switch helper (simplified version) */
static void context_switch(process_t *old_proc, process_t *new_proc) {
    if (old_proc == new_proc) {
        return;
    }
    
    /* Update process states */
    if (old_proc != NULL && old_proc->state == PROC_RUNNING) {
        old_proc->state = PROC_READY;
    }
    
    new_proc->state = PROC_RUNNING;
    
    total_context_switches++;
    
    /* TODO: Actual context switch assembly code would go here */
    /* This would save old_proc's registers and restore new_proc's registers */
    /* For now, this is a simplified version for demonstration */
}

/* Schedule next process */
void scheduler_schedule(void) {
    if (!scheduler_enabled) {
        return;
    }
    
    total_ticks++;
    
    process_t *current = process_current();
    
    /* Decrement time slice of current process */
    if (current != NULL && current->state == PROC_RUNNING) {
        if (current->time_slice > 0) {
            current->time_slice--;
            current->total_time++;
        }
        
        /* If time slice expired, find next process */
        if (current->time_slice == 0) {
            current->time_slice = TIME_SLICE_DEFAULT;
            
            process_t *next = find_next_process();
            
            if (next != NULL && next != current) {
                serial_puts("[SCHED] Switching from ");
                serial_puts(current->name);
                serial_puts(" to ");
                serial_puts(next->name);
                serial_puts("\n");
                
                context_switch(current, next);
            }
        }
    } else {
        /* No current process or current is blocked, find next */
        process_t *next = find_next_process();
        
        if (next != NULL) {
            context_switch(current, next);
        }
    }
}

/* Yield CPU to next process */
void scheduler_yield(void) {
    if (!scheduler_enabled) {
        return;
    }
    
    process_t *current = process_current();
    
    if (current != NULL) {
        /* Reset time slice and find next process */
        current->time_slice = TIME_SLICE_DEFAULT;
        
        process_t *next = find_next_process();
        
        if (next != NULL) {
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
void scheduler_stats(void) {
    char num_str[12];
    
    serial_puts("\n=== Scheduler Statistics ===\n");
    
    serial_puts("Status: ");
    if (scheduler_enabled) {
        serial_puts("ENABLED\n");
    } else {
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
    
    serial_puts("Time slice: ");
    int_to_str(TIME_SLICE_DEFAULT, num_str);
    serial_puts(num_str);
    serial_puts(" ticks\n");
    
    process_t *current = process_current();
    if (current != NULL) {
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
    } else {
        serial_puts("Current process: NONE\n");
    }
    
    serial_puts("\n");
}
