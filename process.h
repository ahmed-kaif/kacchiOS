/* process.h - Process manager interface */
#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

/* Maximum number of processes */
#define MAX_PROCESSES 32

/* Process states */
#define PROC_FREE     0  /* Process slot is available */
#define PROC_READY    1  /* Process is ready to run */
#define PROC_RUNNING  2  /* Process is currently running */
#define PROC_BLOCKED  3  /* Process is blocked (waiting for I/O) */
#define PROC_ZOMBIE   4  /* Process has terminated but not cleaned up */

/* Process priorities */
#define PRIORITY_HIGH   0
#define PRIORITY_NORMAL 1
#define PRIORITY_LOW    2

/* Stack size for each process (4KB) */
#define PROC_STACK_SIZE 4096

/* Process ID type */
typedef int32_t pid_t;

/* Process control block (PCB) */
typedef struct process {
    pid_t pid;                    /* Process ID */
    char name[32];                /* Process name */
    uint32_t state;               /* Current state */
    uint32_t priority;            /* Process priority */
    
    /* CPU context for context switching */
    uint32_t esp;                 /* Stack pointer */
    uint32_t ebp;                 /* Base pointer */
    uint32_t eip;                 /* Instruction pointer */
    uint32_t eflags;              /* CPU flags */
    
    /* Stack information */
    uint32_t stack_base;          /* Base of process stack */
    uint32_t stack_size;          /* Size of stack */
    
    /* Scheduling information */
    uint32_t time_slice;          /* Remaining time slice */
    uint32_t total_time;          /* Total CPU time used */
} process_t;

/* Initialize the process manager */
void process_init(void);

/* Create a new process */
pid_t process_create(const char *name, void (*entry_point)(void), uint32_t priority);

/* Terminate a process */
void process_kill(pid_t pid);

/* Get current running process */
process_t *process_current(void);

/* Get process by PID */
process_t *process_get(pid_t pid);

/* Block current process */
void process_block(void);

/* Unblock a process */
void process_unblock(pid_t pid);

/* Print process table */
void process_list(void);

#endif
