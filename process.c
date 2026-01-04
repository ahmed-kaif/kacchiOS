/* process.c - Process manager implementation */
#include "process.h"
#include "scheduler.h"
#include "memory.h"
#include "serial.h"
#include "string.h"
#include "helper.h"

/* Process table */
process_t process_table[MAX_PROCESSES];

/* Current running process */
pid_t current_pid;

/* Next available PID */
static pid_t next_pid = 0;

/* Helper function to find free process slot */
static int find_free_slot(void)
{
    int i = next_pid % MAX_PROCESSES;
    for (; i < MAX_PROCESSES; i++)
    {
        if (process_table[i].state == PROC_FREE)
        {
            next_pid = (i + 1) % MAX_PROCESSES;
            return i;
        }
        i = i % MAX_PROCESSES;
    }
    return SYSERR;
}

/* Initialize the process manager */
void process_init(void)
{
    int i;

    /* Initialize all process slots as free */
    for (i = 0; i < MAX_PROCESSES; i++)
    {
        process_table[i].pid = -1;
        process_table[i].state = PROC_FREE;
        process_table[i].name[0] = '\0';
        process_table[i].priority = PRIORITY_NORMAL;
        process_table[i].stack_ptr = 0;
        process_table[i].stack_base = 0;
        process_table[i].stack_size = 0;
        process_table[i].time_slice = 0;
        process_table[i].total_time = 0;
        process_table[i].msg = 0;
        process_table[i].has_msg = 0;
    }

    current_pid = -1;
    next_pid = 0;

    serial_puts("Process manager initialized.\n");
}

/* Create a new process */
pid_t process_create(const char *name, void (*entry_point)(void), uint32_t priority)
{
    int slot = find_free_slot();

    if (slot == SYSERR)
    {
        serial_puts("ERROR: Process table full!\n");
        return SYSERR;
    }

    process_t *proc = &process_table[slot];

    /* Assign PID */
    proc->pid = slot;

    /* Copy process name */
    int i;
    for (i = 0; i < 31 && name[i] != '\0'; i++)
    {
        proc->name[i] = name[i];
    }
    proc->name[i] = '\0';

    /* Set priority */
    proc->priority = priority;

    /* Allocate stack using stack allocator */
    proc->stack_size = PROC_STACK_SIZE;
    proc->stack_base = (uint32_t *)stack_alloc(PROC_STACK_SIZE);

    if (proc->stack_base == NULL)
    {
        serial_puts("ERROR: Failed to allocate stack!\n");
        return SYSERR;
    }

    /* Initialize stack pointer (stack grows downward) */
    /* Set up initial stack frame for context switch */
    /* The stack needs to include enough to return from both switch_to_process AND from an interrupt */
    uint32_t *stack_top = (uint32_t *)((uint32_t)proc->stack_base + proc->stack_size);
    
    /* Set up as if the process was interrupted and its context was saved */
    /* This matches what switch_to_process expects when restoring:
     * popfl, popl %ebp, popl %edi, popl %esi, popl %edx, popl %ecx, popl %ebx, ret */
    
    *(--stack_top) = (uint32_t)entry_point; /* Return address (entry point) - for 'ret' */
    *(--stack_top) = 0x00000000;            /* EBX */
    *(--stack_top) = 0x00000000;            /* ECX */
    *(--stack_top) = 0x00000000;            /* EDX */
    *(--stack_top) = 0x00000000;            /* ESI */
    *(--stack_top) = 0x00000000;            /* EDI */
    *(--stack_top) = 0x00000000;            /* EBP */
    *(--stack_top) = 0x00000202;            /* EFLAGS (IF=1) - for 'popfl' */
    
    /* Set the stack pointer to the prepared stack */
    proc->stack_ptr = stack_top;

    /* Initialize scheduling information */
    proc->time_slice = 10; /* Default time slice */
    proc->total_time = 0;

    /* Set state to ready */
    proc->state = PROC_READY;

    serial_puts("Created process: ");
    serial_puts(proc->name);
    serial_puts(" (PID: ");
    char pid_str[12];
    int_to_str(proc->pid, pid_str);
    serial_puts(pid_str);
    serial_puts(")\n");

    return proc->pid;
}

/* Terminate a process */
void process_kill(pid_t pid)
{
    int i;

    for (i = 0; i < MAX_PROCESSES; i++)
    {
        if (process_table[i].pid == pid && process_table[i].state != PROC_FREE)
        {
            /* Free stack memory */
            if (process_table[i].stack_base != NULL)
            {
                stack_free(process_table[i].stack_base, process_table[i].stack_size);
                process_table[i].stack_base = NULL;
            }

            process_table[i].state = PROC_FREE;

            serial_puts("Process ");
            char pid_str[12];
            int_to_str(pid, pid_str);
            serial_puts(pid_str);
            serial_puts(" terminated (stack freed).\n");

            return;
        }
    }

    serial_puts("ERROR: Process not found!\n");
}

/* Get current running process */
process_t *process_current(void)
{
    if (current_pid == -1)
    {
        return NULL;
    }

    return &process_table[current_pid];
}

/* Get Current running process ID */
pid_t process_current_pid(void)
{
    return current_pid;
}

/* Set current running process */
void process_set_current(pid_t pid)
{
    current_pid = pid;
}

/* Get process by PID */
process_t *process_get(pid_t pid)
{
    if (process_is_bad_pid(pid))
    {
        return NULL;
    }
    return &process_table[pid];
}

/* Print process table */
void process_list(void)
{
    int i;
    int count = 0;

    serial_puts("\n=== Process Table ===\n");
    serial_puts("PID  NAME                 STATE      PRIORITY\n");
    serial_puts("---  -------------------  ---------  --------\n");

    for (i = 0; i < MAX_PROCESSES; i++)
    {
        if (process_table[i].state != PROC_FREE)
        {
            char pid_str[12];
            int_to_str(process_table[i].pid, pid_str);

            serial_puts(pid_str);
            serial_puts("    ");
            serial_puts(process_table[i].name);
            serial_puts("    ");

            /* Print state */
            switch (process_table[i].state)
            {
            case PROC_READY:
                serial_puts("READY     ");
                break;
            case PROC_RUNNING:
                serial_puts("RUNNING   ");
                break;
            default:
                serial_puts("UNKNOWN   ");
                break;
            }

            serial_puts(" ");

            /* Print priority */
            switch (process_table[i].priority)
            {
            case PRIORITY_HIGH:
                serial_puts("HIGH");
                break;
            case PRIORITY_NORMAL:
                serial_puts("NORMAL");
                break;
            case PRIORITY_LOW:
                serial_puts("LOW");
                break;
            default:
                serial_puts("?");
                break;
            }

            serial_puts("\n");
            count++;
        }
    }

    serial_puts("\nTotal processes: ");
    char count_str[12];
    int_to_str(count, count_str);
    serial_puts(count_str);
    serial_puts("\n\n");
}

/* Utility functions to get process specific information */

/* Get process name by PID */
const char *process_get_name(pid_t pid)
{
    if (process_is_bad_pid(pid))
    {
        return NULL;
    }
    return process_table[pid].name;
}

/* Get process state by PID */
uint32_t process_get_state(pid_t pid)
{
    if (process_is_bad_pid(pid))
    {
        return PROC_FREE;
    }
    return process_table[pid].state;
}

/* Get process priority by PID */
uint32_t process_get_priority(pid_t pid)
{
    if (process_is_bad_pid(pid))
    {
        return PRIORITY_NORMAL;
    }
    return process_table[pid].priority;
}

/* Get process stack pointer by PID */
uint32_t *process_get_stack_ptr(pid_t pid)
{
    if (process_is_bad_pid(pid))
    {
        return NULL;
    }
    return process_table[pid].stack_ptr;
}

/* Get process stack size by PID */
uint32_t process_get_stack_size(pid_t pid)
{
    if (process_is_bad_pid(pid))
    {
        return 0;
    }
    return process_table[pid].stack_size;
}

/* Count total number of active processes */
int process_count(void)
{
    int i;
    int count = 0;

    for (i = 0; i < MAX_PROCESSES; i++)
    {
        if (process_table[i].state != PROC_FREE)
        {
            count++;
        }
    }

    return count;
}
