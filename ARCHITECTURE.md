# kacchiOS Architecture Overview

## System Component Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                         kacchiOS Kernel                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                   │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐     │
│  │     GDT      │    │     IDT      │    │  PIT Timer   │     │
│  │ (Segments)   │    │ (Interrupts) │    │  (100 Hz)    │     │
│  └──────┬───────┘    └──────┬───────┘    └──────┬───────┘     │
│         │                   │                   │              │
│         │                   │                   │              │
│  ┌──────▼──────────────────▼───────────────────▼───────────┐  │
│  │           Hardware Abstraction Layer (HAL)              │  │
│  │  - Memory Segmentation                                   │  │
│  │  - Exception Handling (CPU Faults)                       │  │
│  │  - IRQ Handling (Hardware Interrupts)                    │  │
│  │  - Timer Ticks                                           │  │
│  └──────────────────────────┬───────────────────────────────┘  │
│                             │                                   │
│  ┌──────────────────────────▼───────────────────────────────┐  │
│  │               Process Manager & Scheduler                 │  │
│  │  - Process Table (32 processes max)                       │  │
│  │  - Round-Robin Scheduling                                 │  │
│  │  - Time Slice Management (10 ticks = 100ms)              │  │
│  │  - Context Switching                                      │  │
│  │  - Preemptive Multitasking                               │  │
│  └──────────────────────────┬───────────────────────────────┘  │
│                             │                                   │
│  ┌──────────────────────────▼───────────────────────────────┐  │
│  │                  Memory Manager                           │  │
│  │  - Fixed-size block allocator (1MB blocks)               │  │
│  │  - kmalloc() / kfree()                                    │  │
│  │  - Stack allocation for processes                         │  │
│  └────────────────────────────────────────────────────────────┘  │
│                                                                   │
└─────────────────────────────────────────────────────────────────┘
```

## Interrupt Flow Diagram

```
Hardware          CPU           Assembly        C Handler        Kernel
  │                │               │                │              │
  │  Timer IRQ0    │               │                │              │
  ├───────────────>│               │                │              │
  │                │  Push State   │                │              │
  │                ├──────────────>│                │              │
  │                │               │  Save Regs     │              │
  │                │               ├───────────────>│              │
  │                │               │                │  Process IRQ │
  │                │               │                ├─────────────>│
  │                │               │                │              │
  │                │               │                │  Call        │
  │                │               │                │  Scheduler   │
  │                │               │                │<─────────────┤
  │                │               │  Send EOI      │              │
  │                │               │<───────────────┤              │
  │                │  Restore Regs │                │              │
  │                │<──────────────┤                │              │
  │                │  IRET         │                │              │
  │<───────────────┤               │                │              │
  │                │               │                │              │
```

## Scheduling Timeline

```
Time (ms):    0    10   20   30   40   50   60   70   80   90  100
             │    │    │    │    │    │    │    │    │    │    │
Timer IRQ:   ▼    ▼    ▼    ▼    ▼    ▼    ▼    ▼    ▼    ▼    ▼
             │    │    │    │    │    │    │    │    │    │    │
Process 1:   ████████████████████████████████████████████████████
                                                    ▲
Process 2:                                          └─── Switch ───> ████████
                                          (time slice expired)

Time Quantum = 10 ticks = 100ms
Each tick = 10ms
```

## Memory Layout

```
High Memory
    │
    ├────────────────────┐
    │   Process Stacks   │  (Dynamically allocated, 4KB each)
    │                    │
    ├────────────────────┤
    │   Heap Blocks      │  (1MB blocks × 32 = 32MB max)
    │   (kmalloc area)   │
    │                    │
    ├────────────────────┤
    │   __kernel_end     │  ← Start of dynamic memory
    │                    │
    ├────────────────────┤
    │   .bss             │  (Uninitialized data: GDT, IDT, TSS)
    │                    │
    ├────────────────────┤
    │   .data            │  (Initialized data)
    │                    │
    ├────────────────────┤
    │   .text            │  (Kernel code)
    │   .rodata          │
    │                    │
    ├────────────────────┤
    │   Multiboot Header │
    │                    │
    └────────────────────┘  0x00100000 (1MB - Kernel load address)
    │
Low Memory
```

## Segment Selectors

```
Selector   Index  Description              Privilege
0x00       0      Null Descriptor          -
0x08       1      Kernel Code Segment      Ring 0
0x10       2      Kernel Data Segment      Ring 0
0x18       3      User Code Segment        Ring 3
0x20       4      User Data Segment        Ring 3
0x28       5      TSS                      -
```

## Interrupt Vector Table (IDT)

```
Vector     Type        Description
0-31       Exception   CPU Exceptions (Division by zero, Page fault, etc.)
32-47      IRQ         Hardware Interrupts (PIC remapped)
  32       IRQ0        PIT Timer ← Used for scheduling
  33       IRQ1        Keyboard
  34-47    IRQ2-15     Other hardware
128        Syscall     System call (INT 0x80)
177        Syscall     Alternative system call (INT 0xB1)
```

## File Organization

```
kacchiOS/
├── boot.S              - Boot assembly (Multiboot, stack setup)
├── kernel.c            - Main kernel entry point
├── gdt.h/c/S          - Global Descriptor Table
├── idt.h/c/S          - Interrupt Descriptor Table
├── timer.h/c          - PIT Timer driver
├── scheduler.h/c      - Process scheduler (now preemptive)
├── process.h/c        - Process manager
├── memory.h/c         - Memory allocator
├── context_switch.S   - Process context switching
├── serial.h/c         - Serial port driver (COM1)
├── string.h/c         - String utilities
├── helper.h/c         - Helper functions (int_to_str, memset)
├── types.h            - Type definitions
├── io.h               - I/O port operations (inb/outb)
├── link.ld            - Linker script
└── Makefile           - Build system
```

## Key Data Structures

### GDT Entry (8 bytes)
```
Bits 0-15:    Limit (low)
Bits 16-31:   Base (low)
Bits 32-39:   Base (middle)
Bits 40-47:   Access byte
Bits 48-51:   Limit (high)
Bits 52-55:   Flags
Bits 56-63:   Base (high)
```

### IDT Entry (8 bytes)
```
Bits 0-15:    Offset (low)
Bits 16-31:   Selector
Bits 32-39:   Always 0
Bits 40-47:   Type attributes
Bits 48-63:   Offset (high)
```

### Process Control Block (PCB)
```c
struct process {
    pid_t pid;              // Process ID
    char name[32];          // Process name
    uint32_t state;         // PROC_FREE/READY/RUNNING
    uint32_t priority;      // Priority level
    uint32_t *stack_ptr;    // Current stack pointer
    uint32_t *stack_base;   // Stack base address
    uint32_t stack_size;    // Stack size (4KB)
    uint32_t time_slice;    // Remaining ticks
    uint32_t total_time;    // Total CPU time used
    umsg32_t msg;           // IPC message
    bool8_t has_msg;        // Message flag
};
```

## System Calls (Future)

```
EAX    Function
0      exit()
1      write()
2      read()
3      open()
4      close()
5      fork()
6      exec()
7      wait()
8      getpid()
9      send_msg()
10     recv_msg()
```

## Performance Characteristics

- **Timer Frequency**: 100 Hz (10ms resolution)
- **Time Quantum**: 10 ticks (100ms per process)
- **Context Switch Overhead**: ~50 CPU cycles
- **Interrupt Latency**: ~200 CPU cycles
- **Max Processes**: 32
- **Stack per Process**: 4 KB
- **Heap Block Size**: 1 MB
- **Max Heap**: 32 MB

## Boot Sequence

1. **BIOS/GRUB** loads kernel at 0x100000
2. **Multiboot** validates kernel image
3. **boot.S** sets up stack, clears BSS
4. **kmain()** initializes:
   - Serial port for debugging
   - GDT (memory segments)
   - IDT (interrupt handlers)
   - PIT timer (100 Hz)
   - Memory manager
   - Process manager
   - Scheduler
5. **Main loop** handles user commands
6. **Scheduler** runs when started

## Debugging Tips

```bash
# Run with serial output
make run

# Run with VGA display
make run-vga

# Debug with GDB
make debug
# In another terminal:
gdb -ex 'target remote localhost:1234' -ex 'symbol-file kernel.elf'

# Set breakpoint at kernel entry
(gdb) break kmain
(gdb) continue

# Examine GDT
(gdb) x/6xw &gdt_entries

# Examine IDT
(gdb) x/256xw &idt_entries

# View current process
(gdb) print *process_current()
```
