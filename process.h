/* process.h - Process manager interface */
#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

/* Maximum number of processes */
#define MAX_PROCESSES 32

/* Process states */
#define PROC_FREE 0    /* Process slot is available */
#define PROC_READY 1   /* Process is ready to run */
#define PROC_RUNNING 2 /* Process is currently running */

/* Process priorities */
#define PRIORITY_HIGH 0
#define PRIORITY_NORMAL 1
#define PRIORITY_LOW 2

/* Stack size for each process (4KB) */
#define PROC_STACK_SIZE 4096

/* check is the pid valid */
#define process_is_bad_pid(pid) ((pid) < 0 || (pid) >= MAX_PROCESSES || process_table[(pid)].state == PROC_FREE)

/* Process ID type */
typedef int32_t pid_t;

/* Process control block (PCB) */
typedef struct process
{
    pid_t pid;         /* Process ID */
    char name[32];     /* Process name */
    uint32_t state;    /* Current state */
    uint32_t priority; /* Process priority */

    /* Stack information */
    uint32_t *stack_ptr;  /* Current stack pointer */
    uint32_t *stack_base; /* Base of process stack */
    uint32_t stack_size;  /* Size of stack */

    /* Scheduling information */
    uint32_t time_slice; /* Remaining time slice */
    uint32_t total_time; /* Total CPU time used */

    /* Message */
    umsg32_t msg;    /* Message for inter-process communication */
    bool8_t has_msg; /* Indicates if there is a message */

} process_t;

/* Current running process */
extern pid_t current_pid;

/* Process table */
extern process_t process_table[MAX_PROCESSES];

/* Initialize the process manager */
void process_init(void);

/* Create a new process */
pid_t process_create(const char *name, void (*entry_point)(void), uint32_t priority);

/* Terminate a process */
void process_kill(pid_t pid);

/* Get current running process */
process_t *process_current(void);

/* Get Current running process ID */
pid_t process_current_pid(void);

/* Set current running process */
void process_set_current(pid_t pid);

/* Get process by PID */
process_t *process_get(pid_t pid);

/* Print process table */
void process_list(void);

/* Utility functions to get process specific information */
const char *process_get_name(pid_t pid);
uint32_t process_get_state(pid_t pid);
uint32_t process_get_priority(pid_t pid);
uint32_t *process_get_stack_ptr(pid_t pid);
uint32_t process_get_stack_size(pid_t pid);
int process_count(void);

#endif
