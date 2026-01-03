# Process Manager and Scheduler Examples

This document provides practical examples of using the process manager and scheduler in kacchiOS.

## Basic Process Creation

### Example 1: Simple Counter Process

```c
/* A process that counts indefinitely */
void counter_process(void) {
    int count = 0;
    
    while (1) {
        serial_puts("Count: ");
        /* Print count value */
        count++;
        
        /* Yield CPU to other processes */
        scheduler_yield();
        
        /* Small delay */
        int i;
        for (i = 0; i < 1000000; i++) {
            __asm__ volatile("nop");
        }
    }
}

/* Create the process */
pid_t pid = process_create("counter", counter_process, PRIORITY_NORMAL);
```

### Example 2: Idle Process

```c
/* A low-priority idle process that runs when nothing else can */
void idle_process(void) {
    while (1) {
        /* Yield immediately */
        scheduler_yield();
        
        /* Halt CPU until next interrupt */
        __asm__ volatile("hlt");
    }
}

/* Create with low priority */
pid_t pid = process_create("idle", idle_process, PRIORITY_LOW);
```

### Example 3: Worker Process

```c
/* A process that performs work units */
void worker_process(void) {
    int work_done = 0;
    
    while (1) {
        /* Simulate work */
        int i;
        for (i = 0; i < 500000; i++) {
            __asm__ volatile("nop");
        }
        
        work_done++;
        
        /* Periodically report progress */
        if (work_done % 100 == 0) {
            serial_puts("Worker: Completed 100 units\n");
        }
        
        /* Yield to scheduler */
        scheduler_yield();
    }
}

/* Create the worker */
pid_t pid = process_create("worker", worker_process, PRIORITY_NORMAL);
```

## Process Management Examples

### Example 4: Creating Multiple Processes

```c
void kmain(void) {
    /* Initialize subsystems */
    serial_init();
    memory_init();
    process_init();
    scheduler_init();
    
    /* Create multiple processes */
    process_create("idle", idle_process, PRIORITY_LOW);
    process_create("worker1", worker_process, PRIORITY_NORMAL);
    process_create("worker2", worker_process, PRIORITY_NORMAL);
    process_create("counter", counter_process, PRIORITY_HIGH);
    
    /* Start scheduler */
    scheduler_start();
    
    /* Main kernel loop */
    while (1) {
        /* Handle user input */
    }
}
```

### Example 5: Process Termination

```c
/* Create a process that terminates after some work */
void temporary_process(void) {
    int iterations = 0;
    
    while (iterations < 1000) {
        /* Do work */
        iterations++;
        scheduler_yield();
    }
    
    /* Process terminates naturally by returning */
    serial_puts("Temporary process finished!\n");
}

/* In main code */
pid_t temp_pid = process_create("temp", temporary_process, PRIORITY_NORMAL);

/* Later, kill it manually if needed */
process_kill(temp_pid);
```

### Example 6: Blocking and Unblocking

```c
/* Global variable to track blocked process */
pid_t blocked_pid = -1;

/* Process that can be blocked */
void blockable_process(void) {
    while (1) {
        serial_puts("Running...\n");
        
        /* Check if we should block */
        /* In real system, this would be waiting for I/O */
        
        scheduler_yield();
    }
}

/* In main code or another process */
void block_example(void) {
    /* Create process */
    blocked_pid = process_create("blockable", blockable_process, PRIORITY_NORMAL);
    
    /* Later, block it */
    process_block();  /* Blocks current process */
    
    /* Or unblock a specific process */
    process_unblock(blocked_pid);
}
```

## Scheduler Examples

### Example 7: Manual Scheduling

```c
/* Manually trigger scheduler (useful for testing) */
void test_scheduler(void) {
    /* Create some processes */
    process_create("proc1", worker_process, PRIORITY_NORMAL);
    process_create("proc2", worker_process, PRIORITY_NORMAL);
    
    /* Start scheduler */
    scheduler_start();
    
    /* Manually call scheduler multiple times */
    int i;
    for (i = 0; i < 100; i++) {
        scheduler_schedule();
        
        /* Simulate timer tick delay */
        int j;
        for (j = 0; j < 100000; j++) {
            __asm__ volatile("nop");
        }
    }
    
    /* Check statistics */
    scheduler_stats();
}
```

### Example 8: Cooperative Multitasking

```c
/* Process that cooperates by yielding frequently */
void cooperative_process(void) {
    while (1) {
        /* Do a small amount of work */
        serial_putc('.');
        
        /* Immediately yield to be fair */
        scheduler_yield();
    }
}

/* Process that hogs CPU (bad example) */
void cpu_hog_process(void) {
    while (1) {
        /* Do lots of work without yielding */
        int i;
        for (i = 0; i < 10000000; i++) {
            __asm__ volatile("nop");
        }
        
        /* Finally yield */
        scheduler_yield();
    }
}
```

## Interactive Shell Examples

### Example 9: Command Handler

```c
void handle_command(char *input) {
    if (strcmp(input, "ps") == 0) {
        /* List all processes */
        process_list();
        
    } else if (strcmp(input, "stats") == 0) {
        /* Show scheduler statistics */
        scheduler_stats();
        
    } else if (strcmp(input, "create") == 0) {
        /* Create a new worker process */
        static int worker_count = 0;
        char name[32];
        
        /* Generate unique name */
        strcpy(name, "worker");
        /* Append number (simplified) */
        
        pid_t pid = process_create(name, worker_process, PRIORITY_NORMAL);
        
        serial_puts("Created process with PID: ");
        /* Print PID */
        serial_puts("\n");
        
        worker_count++;
        
    } else if (strcmp(input, "help") == 0) {
        serial_puts("Available commands:\n");
        serial_puts("  ps       - List processes\n");
        serial_puts("  stats    - Show statistics\n");
        serial_puts("  create   - Create new worker\n");
        serial_puts("  help     - Show this help\n");
        
    } else {
        serial_puts("Unknown command. Type 'help' for available commands.\n");
    }
}
```

## Advanced Examples

### Example 10: Priority-Based Process Creation

```c
void create_priority_processes(void) {
    /* High priority - critical system tasks */
    process_create("watchdog", watchdog_process, PRIORITY_HIGH);
    process_create("interrupt_handler", interrupt_process, PRIORITY_HIGH);
    
    /* Normal priority - regular tasks */
    process_create("user_app1", user_app_process, PRIORITY_NORMAL);
    process_create("user_app2", user_app_process, PRIORITY_NORMAL);
    
    /* Low priority - background tasks */
    process_create("logger", logger_process, PRIORITY_LOW);
    process_create("idle", idle_process, PRIORITY_LOW);
}
```

### Example 11: Process Communication (Simplified)

```c
/* Shared data structure for simple communication */
typedef struct {
    int data;
    int ready;
} shared_data_t;

shared_data_t shared;

/* Producer process */
void producer_process(void) {
    int value = 0;
    
    while (1) {
        /* Wait until consumer is ready */
        while (shared.ready) {
            scheduler_yield();
        }
        
        /* Produce data */
        shared.data = value++;
        shared.ready = 1;
        
        serial_puts("Produced: ");
        /* Print value */
        serial_puts("\n");
        
        scheduler_yield();
    }
}

/* Consumer process */
void consumer_process(void) {
    while (1) {
        /* Wait for data */
        while (!shared.ready) {
            scheduler_yield();
        }
        
        /* Consume data */
        int value = shared.data;
        shared.ready = 0;
        
        serial_puts("Consumed: ");
        /* Print value */
        serial_puts("\n");
        
        scheduler_yield();
    }
}

/* Create both processes */
void setup_producer_consumer(void) {
    shared.data = 0;
    shared.ready = 0;
    
    process_create("producer", producer_process, PRIORITY_NORMAL);
    process_create("consumer", consumer_process, PRIORITY_NORMAL);
}
```

### Example 12: Process Monitoring

```c
/* Monitor process that periodically checks system state */
void monitor_process(void) {
    int check_interval = 0;
    
    while (1) {
        check_interval++;
        
        /* Every 1000 iterations, print status */
        if (check_interval >= 1000) {
            serial_puts("\n=== System Monitor ===\n");
            process_list();
            scheduler_stats();
            serial_puts("======================\n\n");
            
            check_interval = 0;
        }
        
        scheduler_yield();
    }
}

/* Create monitor */
pid_t monitor_pid = process_create("monitor", monitor_process, PRIORITY_LOW);
```

## Testing Examples

### Example 13: Stress Test

```c
/* Create many processes to test limits */
void stress_test(void) {
    int i;
    int created = 0;
    
    serial_puts("Starting stress test...\n");
    
    for (i = 0; i < MAX_PROCESSES; i++) {
        char name[32];
        /* Generate name: "stress_0", "stress_1", etc. */
        
        pid_t pid = process_create(name, worker_process, PRIORITY_NORMAL);
        
        if (pid != -1) {
            created++;
        } else {
            serial_puts("Process table full!\n");
            break;
        }
    }
    
    serial_puts("Created ");
    /* Print created count */
    serial_puts(" processes\n");
    
    /* List all processes */
    process_list();
}
```

### Example 14: Scheduler Fairness Test

```c
/* Track how much CPU time each process gets */
int process_counters[3] = {0, 0, 0};

void test_process_0(void) {
    while (1) {
        process_counters[0]++;
        scheduler_yield();
    }
}

void test_process_1(void) {
    while (1) {
        process_counters[1]++;
        scheduler_yield();
    }
}

void test_process_2(void) {
    while (1) {
        process_counters[2]++;
        scheduler_yield();
    }
}

void fairness_test(void) {
    /* Create test processes */
    process_create("test0", test_process_0, PRIORITY_NORMAL);
    process_create("test1", test_process_1, PRIORITY_NORMAL);
    process_create("test2", test_process_2, PRIORITY_NORMAL);
    
    /* Start scheduler */
    scheduler_start();
    
    /* Run for a while */
    int i;
    for (i = 0; i < 10000; i++) {
        scheduler_schedule();
    }
    
    /* Print results */
    serial_puts("Process 0 count: ");
    /* Print process_counters[0] */
    serial_puts("\nProcess 1 count: ");
    /* Print process_counters[1] */
    serial_puts("\nProcess 2 count: ");
    /* Print process_counters[2] */
    serial_puts("\n");
    
    /* Counts should be roughly equal for fair scheduling */
}
```

## Best Practices

### 1. Always Yield
Processes should call `scheduler_yield()` regularly to allow other processes to run.

### 2. Appropriate Priorities
- Use HIGH priority sparingly for critical tasks
- Most processes should be NORMAL priority
- Use LOW priority for background/idle tasks

### 3. Clean Termination
Processes should either run forever or terminate cleanly by returning from their entry function.

### 4. Avoid Busy Waiting
Instead of tight loops checking conditions, yield the CPU:
```c
/* Bad */
while (!condition) { }

/* Good */
while (!condition) {
    scheduler_yield();
}
```

### 5. Initialize Before Starting
Always initialize process manager and scheduler before creating processes:
```c
process_init();
scheduler_init();
/* Now create processes */
process_create(...);
scheduler_start();
```

## Common Pitfalls

1. **Forgetting to yield** - Process hogs CPU
2. **Creating too many processes** - Exceeds MAX_PROCESSES limit
3. **Not checking return values** - process_create() can fail
4. **Starting scheduler before creating processes** - Nothing to run
5. **Blocking without unblocking** - Process stuck forever

## Next Steps

To fully implement the scheduler:

1. Add timer interrupt handler
2. Implement full context switching in assembly
3. Add process synchronization primitives
4. Implement system calls
5. Add memory protection
6. Implement inter-process communication

See PROCESS_SCHEDULER_GUIDE.md for detailed implementation guidance.
