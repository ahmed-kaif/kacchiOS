# kacchiOS Hardware Interrupt Implementation

## Overview
This document describes the hardware-based interrupt system, GDT, PIT timer, and preemptive multitasking implementation added to kacchiOS.

## Components Implemented

### 1. GDT (Global Descriptor Table)
**Files:** `gdt.h`, `gdt.c`, `gdt.S`

The GDT defines memory segments for the x86 processor. Our implementation includes:
- **Null Descriptor (0x00)**: Required by x86 architecture
- **Kernel Code Segment (0x08)**: Executable code with ring 0 privilege
- **Kernel Data Segment (0x10)**: Read/write data with ring 0 privilege
- **User Code Segment (0x18)**: Executable code with ring 3 privilege (for future user mode)
- **User Data Segment (0x20)**: Read/write data with ring 3 privilege (for future user mode)
- **TSS (Task State Segment) (0x28)**: For hardware task switching

**Key Functions:**
- `gdt_init()` - Initializes and loads the GDT
- `gdt_set_gate()` - Sets up a GDT descriptor
- `gdt_write_tss()` - Configures the TSS for kernel stack switching
- `gdt_flush()` - Assembly routine to load GDT using LGDT instruction
- `tss_flush()` - Assembly routine to load TSS using LTR instruction

### 2. IDT (Interrupt Descriptor Table)
**Files:** `idt.h`, `idt.c`, `idt.S`

The IDT handles both CPU exceptions and hardware interrupts (IRQs).

**CPU Exceptions (0-31):**
- Division by zero, page faults, general protection faults, etc.
- Handled by `isr_handler()` which displays error message and halts

**Hardware Interrupts (32-47):**
- IRQ0: PIT Timer (used for preemptive multitasking)
- IRQ1: Keyboard
- IRQ3-15: Other hardware devices
- Handled by `irq_handler()` which calls registered handlers

**PIC Remapping:**
The 8259 PIC (Programmable Interrupt Controller) is remapped to avoid conflicts:
- Master PIC (IRQ 0-7) → Interrupts 32-39
- Slave PIC (IRQ 8-15) → Interrupts 40-47

**Key Functions:**
- `idt_init()` - Initializes IDT, remaps PIC, sets up all interrupt gates
- `idt_set_gate()` - Configures an IDT entry
- `irq_install_handler()` - Registers a custom IRQ handler
- `irq_uninstall_handler()` - Removes an IRQ handler
- `isr_handler()` - C handler for CPU exceptions
- `irq_handler()` - C handler for hardware interrupts
- `idt_flush()` - Assembly routine to load IDT using LIDT and enable interrupts (STI)

**Assembly Stubs:**
- ISR stubs for each exception (with/without error codes)
- IRQ stubs for each hardware interrupt
- Common stub handlers that save/restore CPU state

### 3. PIT Timer (Programmable Interval Timer)
**Files:** `timer.h`, `timer.c`

The PIT generates periodic interrupts for preemptive multitasking.

**Configuration:**
- Frequency: 100 Hz (10ms per tick)
- PIT base frequency: 1.193182 MHz
- Channel 0, Mode 3 (Square Wave Generator)

**Key Functions:**
- `timer_init()` - Programs the PIT and installs timer IRQ handler
- `timer_get_ticks()` - Returns elapsed ticks since boot
- `timer_wait()` - Busy-wait for specified number of ticks
- `timer_handler()` - IRQ0 handler that increments tick counter and calls scheduler

### 4. Preemptive Multitasking Integration
**Modified Files:** `scheduler.c`, `scheduler.h`, `kernel.c`

The scheduler now works with hardware timer interrupts for true preemptive multitasking:

**New Function:**
- `scheduler_tick()` - Called by timer interrupt to perform context switches

**Flow:**
1. Timer fires IRQ0 every 10ms
2. `timer_handler()` increments tick counter
3. `timer_handler()` calls `scheduler_tick()`
4. `scheduler_tick()` calls `scheduler_schedule()`
5. `scheduler_schedule()` checks time slices and switches processes if needed

### 5. Helper Utilities
**Modified Files:** `helper.h`, `helper.c`

Added `memset()` function for memory initialization, used by GDT and IDT implementations.

## Initialization Sequence

The kernel initialization in `kmain()` follows this order:

1. **Serial Port** - For debug output
2. **GDT** - Set up memory segmentation
3. **IDT** - Set up interrupt handling
4. **PIT Timer** - Start periodic interrupts
5. **Memory Manager** - Initialize heap allocator
6. **Process Manager** - Set up process table
7. **Scheduler** - Initialize round-robin scheduler

## Build System Updates

The Makefile was updated to compile and link the new files:
- `gdt.o` and `gdt_asm.o` (from gdt.c and gdt.S)
- `idt.o` and `idt_asm.o` (from idt.c and idt.S)
- `timer.o` (from timer.c)

## Technical Details

### Interrupt Handling Flow

1. **Hardware Interrupt Occurs:**
   - CPU pushes SS, ESP, EFLAGS, CS, EIP onto stack
   - CPU jumps to IDT entry

2. **Assembly Stub (idt.S):**
   - Pushes error code (or dummy 0) and interrupt number
   - Pushes all registers (PUSHA)
   - Pushes segment registers
   - Loads kernel data segments
   - Calls C handler (`isr_handler` or `irq_handler`)

3. **C Handler (idt.c):**
   - Processes the interrupt
   - Calls registered handlers (for IRQs)
   - Sends EOI (End of Interrupt) to PIC

4. **Return to Stub:**
   - Restores segment registers
   - Restores all registers (POPA)
   - Re-enables interrupts (STI)
   - Returns from interrupt (IRET)

### Context Switching with Interrupts

The timer interrupt enables true preemptive multitasking:
- Processes are interrupted automatically every 10ms
- No need for manual `scheduler_yield()` calls
- Fair CPU time distribution via round-robin scheduling

## Testing

To test the implementation:

```bash
make clean
make
make run
```

You should see:
- GDT initialization message
- IDT initialization message
- PIC remap confirmation
- PIT timer frequency
- All subsequent kernel messages

The scheduler will automatically switch between processes when you type `start` at the prompt.

## Future Enhancements

- **Keyboard Driver**: Handle IRQ1 for keyboard input
- **System Calls**: Use INT 0x80 for user space to kernel communication
- **User Mode**: Utilize ring 3 segments for protected user processes
- **Virtual Memory**: Add paging support with page fault handler
- **More IRQ Handlers**: Support additional hardware devices

## References

- Intel 64 and IA-32 Architectures Software Developer's Manual
- OSDev Wiki: https://wiki.osdev.org/
- 8259 PIC Datasheet
- 8253/8254 PIT Datasheet

## Author

Implementation based on JazzOS reference and adapted for kacchiOS.
Date: January 4, 2026
