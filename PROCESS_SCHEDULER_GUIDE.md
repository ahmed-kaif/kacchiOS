# Process Manager and Scheduler Implementation Guide

## Overview

This document describes the process manager and scheduler implementation for kacchiOS. The system implements a simple round-robin scheduler with process control blocks (PCBs) and basic process management.

## Architecture

### Components

1. **Process Manager** (`process.c`, `process.h`)
   - Manages process lifecycle
   - Maintains process table
   - Handles process creation and termination

2. **Scheduler** (`scheduler.c`, `scheduler.h`)
   - Implements round-robin scheduling algorithm
   - Manages CPU time allocation
   - Handles context switching (framework)

## Process Manager

### Data Structures

#### Process Control Block (PCB)
```c
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
```

### Process States

- `PROC_FREE` - Process slot is available
- `PROC_READY` - Process is ready to run
- `PROC_RUNNING` - Process is currently running
- `PROC_BLOCKED` - Process is blocked (waiting for I/O)
- `PROC_ZOMBIE` - Process has terminated but not cleaned up

### Process Priorities

- `PRIORITY_HIGH` (0) - High priority processes
- `PRIORITY_NORMAL` (1) - Normal priority processes
- `PRIORITY_LOW` (2) - Low priority processes

### Key Functions

#### `void process_init(void)`
Initializes the process manager by clearing the process table and setting up initial state.

#### `pid_t process_create(const char *name, void (*entry_point)(void), uint32_t priority)`
Creates a new process with the given name, entry point function, and priority.

**Parameters:**
- `name` - Process name (max 31 characters)
- `entry_point` - Function pointer to process entry point
- `priority` - Process priority level

**Returns:** Process ID (PID) or -1 on failure

**Implementation Details:**
- Allocates a 4KB stack for the process
- Initializes CPU context (ESP, EBP, EIP, EFLAGS)
- Sets process state to READY
- Assigns unique PID

#### `void process_kill(pid_t pid)`
Terminates a process by setting its state to ZOMBIE.

#### `process_t *process_current(void)`
Returns pointer to currently running process or NULL if none.

#### `process_t *process_get(pid_t pid)`
Returns pointer to process with given PID or NULL if not found.

#### `void process_block(void)`
Blocks the current process (sets state to BLOCKED).

#### `void process_unblock(pid_t pid)`
Unblocks a blocked process (sets state to READY).

#### `void process_list(void)`
Prints formatted process table showing all active processes.

## Scheduler

### Scheduling Algorithm

The scheduler implements a **Round-Robin** algorithm with time slicing:

1. Each process gets a fixed time slice (default: 10 ticks)
2. When time slice expires, scheduler switches to next ready process
3. Processes are selected in circular order from process table
4. Blocked and zombie processes are skipped

### Key Functions

#### `void scheduler_init(void)`
Initializes the scheduler and resets statistics.

#### `void scheduler_start(void)`
Starts the scheduler and begins multitasking by finding the first ready process.

#### `void scheduler_schedule(void)`
Main scheduling function called by timer interrupt (or manually for demo).

**Algorithm:**
1. Increment total tick counter
2. Decrement current process time slice
3. If time slice expired:
   - Reset time slice to default
   - Find next ready process
   - Perform context switch if different process found
4. If no current process or current is blocked:
   - Find next ready process
   - Perform context switch

#### `void scheduler_yield(void)`
Allows current process to voluntarily yield CPU to next ready process.

#### `void scheduler_stats(void)`
Prints scheduler statistics including:
- Scheduler status (enabled/disabled)
- Total ticks
- Context switches
- Current process information
- Time slice information

### Context Switching

The current implementation provides a **framework** for context switching:

```c
static void context_switch(process_t *old_proc, process_t *new_proc)
```

**Current Implementation:**
- Updates process states
- Tracks context switch count
- Placeholder for actual register save/restore

**Full Implementation Would Include:**
1. Save old process registers (EAX, EBX, ECX, EDX, ESI, EDI, ESP, EBP, EIP, EFLAGS)
2. Save old process stack pointer
3. Switch to new process stack
4. Restore new process registers
5. Jump to new process instruction pointer

**Assembly Code Required:**
```asm
; Save current process context
push eax
push ebx
push ecx
push edx
push esi
push edi
push ebp
pushf

; Save ESP to old process PCB
mov [old_proc->esp], esp

; Load new process ESP
mov esp, [new_proc->esp]

; Restore new process context
popf
pop ebp
pop edi
pop esi
pop edx
pop ecx
pop ebx
pop eax

; Jump to new process
jmp [new_proc->eip]
```

## Integration with Kernel

### Initialization Sequence

```c
void kmain(void) {
    /* Initialize subsystems */
    serial_init();
    memory_init();
    process_init();      /* Initialize process manager */
    scheduler_init();    /* Initialize scheduler */
    
    /* Create processes */
    process_create("idle", process_idle, PRIORITY_LOW);
    process_create("worker", process_worker, PRIORITY_NORMAL);
    
    /* Start scheduler */
    scheduler_start();
    
    /* Main kernel loop */
    while (1) {
        /* Handle commands */
    }
}
```

### Example Processes

#### Idle Process
```c
void process_idle(void) {
    while (1) {
        scheduler_yield();
        __asm__ volatile("hlt");
    }
}
```

#### Worker Process
```c
void process_worker(void) {
    int work_units = 0;
    while (1) {
        work_units++;
        /* Do work */
        scheduler_yield();
    }
}
```

## Usage

### Building

```bash
make clean
make
```

### Running

```bash
make run
```

### Available Commands

When running kacchiOS:

- `ps` - List all processes with their states and priorities
- `stats` - Show scheduler statistics
- `help` - Display help message
- `exit` - Halt the system

### Example Output

```
========================================
    kacchiOS - Minimal Baremetal OS
========================================
Hello from kacchiOS!
Process Manager & Scheduler Demo

Memory Test: Memory init success

Process manager initialized.
Scheduler initialized (Round-Robin).
Creating demo processes...
Created process: idle (PID: 0)
Created process: worker1 (PID: 1)
Created process: counter (PID: 2)

kacchiOS> ps

=== Process Table ===
PID  NAME                 STATE      PRIORITY
---  -------------------  ---------  --------
0    idle                 READY      LOW
1    worker1              READY      NORMAL
2    counter              READY      NORMAL

Total processes: 3

kacchiOS> stats

=== Scheduler Statistics ===
Status: DISABLED
Total ticks: 0
Context switches: 0
Time slice: 10 ticks
Current process: NONE
```

## Future Enhancements

### 1. Complete Context Switching
Implement full assembly-level context switching with register save/restore.

### 2. Timer Interrupt Integration
Connect scheduler to hardware timer interrupt for automatic preemption.

### 3. Priority Scheduling
Enhance scheduler to respect process priorities (currently round-robin ignores priority).

### 4. Process Synchronization
Add mutexes, semaphores, and condition variables.

### 5. Inter-Process Communication (IPC)
Implement message passing or shared memory.

### 6. Process Sleep/Wake
Add ability for processes to sleep for specified time.

### 7. Memory Management Integration
Implement proper memory deallocation when processes terminate.

### 8. System Calls
Add system call interface for process management operations.

## Design Decisions

### Why Round-Robin?
- Simple to implement and understand
- Fair CPU allocation
- Good for educational purposes
- No starvation issues

### Why Fixed Time Slice?
- Predictable behavior
- Easy to reason about
- Suitable for demonstration
- Can be tuned based on workload

### Why Static Process Table?
- Simple memory management
- Predictable memory usage
- No dynamic allocation overhead
- Suitable for embedded/baremetal systems

## Testing

### Test Scenarios

1. **Process Creation**
   - Create multiple processes
   - Verify unique PIDs
   - Check process table entries

2. **Process Listing**
   - Use `ps` command
   - Verify all processes shown
   - Check state and priority display

3. **Scheduler Statistics**
   - Use `stats` command
   - Verify tick counting
   - Check context switch tracking

4. **Process States**
   - Verify READY state after creation
   - Check state transitions
   - Test ZOMBIE state after kill

## Troubleshooting

### Process Not Created
- Check if process table is full (MAX_PROCESSES = 32)
- Verify memory manager has space for stack allocation
- Check entry point function is valid

### Scheduler Not Running
- Ensure `scheduler_start()` is called
- Verify at least one process is in READY state
- Check scheduler_enabled flag

### Context Switch Not Working
- Current implementation is framework only
- Full context switching requires assembly code
- Timer interrupt integration needed for automatic preemption

## References

- [OSDev Wiki - Processes](https://wiki.osdev.org/Processes)
- [OSDev Wiki - Scheduling](https://wiki.osdev.org/Scheduling_Algorithms)
- [XINU Operating System](https://xinu.cs.purdue.edu/)
- [Operating Systems: Three Easy Pieces](https://pages.cs.wisc.edu/~remzi/OSTEP/)

## License

This implementation is part of kacchiOS and is licensed under the MIT License.
