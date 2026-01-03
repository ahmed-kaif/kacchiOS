/* process.c - Process manager implementation */
#include "process.h"
#include "scheduler.h"
#include "memory.h"
#include "serial.h"
#include "string.h"

/* Process table */
static process_t process_table[MAX_PROCESSES];

/* Current running process */
static pid_t current_pid = -1;

/* Next available PID */
static pid_t next_pid = 0;

/* Helper function to find free process slot */
static int find_free_slot(void) {
    int i;
    for (i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROC_FREE) {
            return i;
        }
    }
    return -1;
}

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

/* Initialize the process manager */
void process_init(void) {
    int i;
    
    /* Initialize all process slots as free */
    for (i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid = -1;
        process_table[i].state = PROC_FREE;
        process_table[i].name[0] = '\0';
        process_table[i].priority = PRIORITY_NORMAL;
        process_table[i].esp = 0;
        process_table[i].ebp = 0;
        process_table[i].eip = 0;
        process_table[i].eflags = 0;
        process_table[i].stack_base = 0;
        process_table[i].stack_size = 0;
        process_table[i].time_slice = TIME_SLICE_DEFAULT;
        process_table[i].total_time = 0;
    }
    
    current_pid = -1;
    next_pid = 0;
    
    serial_puts("Process manager initialized.\n");
}

/* Create a new process */
pid_t process_create(const char *name, void (*entry_point)(void), uint32_t priority) {
    int slot = find_free_slot();
    
    if (slot == -1) {
        serial_puts("ERROR: Process table full!\n");
        return -1;
    }
    
    process_t *proc = &process_table[slot];
    
    /* Assign PID */
    proc->pid = next_pid++;
    
    /* Copy process name */
    int i;
    for (i = 0; i < 31 && name[i] != '\0'; i++) {
        proc->name[i] = name[i];
    }
    proc->name[i] = '\0';
    
    /* Set priority */
    proc->priority = priority;
    
    /* Allocate stack */
    proc->stack_size = PROC_STACK_SIZE;
    proc->stack_base = (uint32_t)kmalloc(PROC_STACK_SIZE);
    
    /* Initialize stack pointer (stack grows downward) */
    proc->esp = proc->stack_base + PROC_STACK_SIZE - 4;
    proc->ebp = proc->esp;
    
    /* Set entry point */
    proc->eip = (uint32_t)entry_point;
    
    /* Set default flags (interrupts enabled) */
    proc->eflags = 0x202;
    
    /* Initialize scheduling info */
    proc->time_slice = TIME_SLICE_DEFAULT;
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
void process_kill(pid_t pid) {
    int i;
    
    for (i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == pid && process_table[i].state != PROC_FREE) {
            process_table[i].state = PROC_ZOMBIE;
            
            serial_puts("Process ");
            char pid_str[12];
            int_to_str(pid, pid_str);
            serial_puts(pid_str);
            serial_puts(" terminated.\n");
            
            /* TODO: Free stack memory when memory manager supports free() */
            
            return;
        }
    }
    
    serial_puts("ERROR: Process not found!\n");
}

/* Get current running process */
process_t *process_current(void) {
    if (current_pid == -1) {
        return NULL;
    }
    
    int i;
    for (i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == current_pid) {
            return &process_table[i];
        }
    }
    
    return NULL;
}

/* Get process by PID */
process_t *process_get(pid_t pid) {
    int i;
    
    for (i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == pid && process_table[i].state != PROC_FREE) {
            return &process_table[i];
        }
    }
    
    return NULL;
}

/* Block current process */
void process_block(void) {
    process_t *proc = process_current();
    
    if (proc != NULL) {
        proc->state = PROC_BLOCKED;
        serial_puts("Process blocked.\n");
    }
}

/* Unblock a process */
void process_unblock(pid_t pid) {
    process_t *proc = process_get(pid);
    
    if (proc != NULL && proc->state == PROC_BLOCKED) {
        proc->state = PROC_READY;
        serial_puts("Process unblocked.\n");
    }
}

/* Print process table */
void process_list(void) {
    int i;
    int count = 0;
    
    serial_puts("\n=== Process Table ===\n");
    serial_puts("PID  NAME                 STATE      PRIORITY\n");
    serial_puts("---  -------------------  ---------  --------\n");
    
    for (i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state != PROC_FREE) {
            char pid_str[12];
            int_to_str(process_table[i].pid, pid_str);
            
            serial_puts(pid_str);
            serial_puts("    ");
            serial_puts(process_table[i].name);
            serial_puts("    ");
            
            /* Print state */
            switch (process_table[i].state) {
                case PROC_READY:
                    serial_puts("READY     ");
                    break;
                case PROC_RUNNING:
                    serial_puts("RUNNING   ");
                    break;
                case PROC_BLOCKED:
                    serial_puts("BLOCKED   ");
                    break;
                case PROC_ZOMBIE:
                    serial_puts("ZOMBIE    ");
                    break;
                default:
                    serial_puts("UNKNOWN   ");
                    break;
            }
            
            serial_puts(" ");
            
            /* Print priority */
            switch (process_table[i].priority) {
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
