# kacchiOS Quick Reference Card

## 🚀 Quick Start

```bash
# Build
make clean && make

# Run
make run

# Debug
make debug
```

## 📋 Command Overview

| Command | Description |
|---------|-------------|
| `ps` | List all processes |
| `start` | Start the scheduler (enables preemptive multitasking) |
| `stats` | Show scheduler statistics |
| `mem` | Show memory statistics |
| `help` | Show available commands |
| `exit` | Halt the system |

## 🔧 New Components

### GDT (Global Descriptor Table)
- **Files**: `gdt.h`, `gdt.c`, `gdt.S`
- **Function**: `gdt_init()`
- **Features**: 6 segments (null, kernel code/data, user code/data, TSS)

### IDT (Interrupt Descriptor Table)
- **Files**: `idt.h`, `idt.c`, `idt.S`
- **Function**: `idt_init()`
- **Features**: 256 vectors, PIC remapping, exception/IRQ handlers

### PIT Timer
- **Files**: `timer.h`, `timer.c`
- **Function**: `timer_init()`
- **Features**: 100 Hz (10ms), IRQ0, preemptive scheduling

## 🎯 Key Functions

### GDT Functions
```c
void gdt_init(void);                          // Initialize GDT
void gdt_set_gate(num, base, limit, access);  // Set GDT entry
void gdt_write_tss(num, ss0, esp0);          // Configure TSS
```

### IDT Functions
```c
void idt_init(void);                         // Initialize IDT
void idt_set_gate(num, base, sel, flags);    // Set IDT entry
void irq_install_handler(irq, handler);      // Register IRQ handler
void irq_uninstall_handler(irq);             // Remove IRQ handler
```

### Timer Functions
```c
void timer_init(void);                       // Initialize PIT
uint32_t timer_get_ticks(void);             // Get tick count
void timer_wait(uint32_t ticks);            // Wait for ticks
```

### Scheduler Functions
```c
void scheduler_init(void);                   // Initialize scheduler
void scheduler_start(void);                  // Start scheduler
void scheduler_tick(void);                   // Timer tick handler ✨ NEW
void scheduler_yield(void);                  // Manual yield
```

## 💾 Memory Layout

```
Address       Description
0x00100000    Kernel load address (1MB)
0x00XXXXXX    Kernel code/data
0x01085440    Heap start (__kernel_end)
0x????????    Process stacks (4KB each)
```

## 🔢 Important Numbers

| Item | Value |
|------|-------|
| Timer Frequency | 100 Hz (10ms per tick) |
| Time Quantum | 10 ticks (100ms) |
| Max Processes | 32 |
| Stack Size | 4 KB |
| Heap Block Size | 1 MB |
| Max Heap | 32 MB |

## 🎨 Segment Selectors

| Selector | Description |
|----------|-------------|
| 0x00 | Null descriptor |
| 0x08 | Kernel code (Ring 0) |
| 0x10 | Kernel data (Ring 0) |
| 0x18 | User code (Ring 3) |
| 0x20 | User data (Ring 3) |
| 0x28 | TSS |

## ⚡ Interrupt Vectors

| Vector | Type | Description |
|--------|------|-------------|
| 0-31 | Exception | CPU exceptions |
| 32 | IRQ0 | PIT Timer ⏰ |
| 33 | IRQ1 | Keyboard |
| 34-47 | IRQ2-15 | Other hardware |
| 128 | Syscall | System call (INT 0x80) |

## 📊 Initialization Order

1. Serial port (`serial_init()`)
2. GDT (`gdt_init()`) ✨
3. IDT (`idt_init()`) ✨
4. Timer (`timer_init()`) ✨
5. Memory (`memory_init()`)
6. Processes (`process_init()`)
7. Scheduler (`scheduler_init()`)

## 🐛 Debugging Tips

```bash
# View GDT
(gdb) x/6xw &gdt_entries

# View IDT
(gdb) x/256xw &idt_entries

# View current process
(gdb) print *process_current()

# View timer ticks
(gdb) print timer_ticks

# Set breakpoint at ISR
(gdb) break isr_handler
(gdb) break irq_handler
```

## 📁 File Structure

```
Core Files:
  kernel.c          - Main kernel
  boot.S            - Boot code
  link.ld           - Linker script
  Makefile          - Build system

Hardware:
  gdt.h/c/S         - Segmentation ✨
  idt.h/c/S         - Interrupts ✨
  timer.h/c         - PIT Timer ✨
  serial.h/c        - Serial port
  io.h              - I/O operations

System:
  scheduler.h/c     - Process scheduler
  process.h/c       - Process manager
  memory.h/c        - Memory manager
  context_switch.S  - Context switching

Utilities:
  helper.h/c        - Helper functions
  string.h/c        - String operations
  types.h           - Type definitions

Documentation:
  IMPLEMENTATION_SUMMARY.md      - Summary
  INTERRUPT_IMPLEMENTATION.md    - Details
  ARCHITECTURE.md                - Architecture
  QUICK_REFERENCE.md             - This file
```

## ⚙️ Build Flags

```
CFLAGS:
  -m32              32-bit mode
  -ffreestanding    Freestanding environment
  -O2               Optimization level 2
  -nostdinc         No standard includes
  -fno-builtin      No compiler builtins
  
ASFLAGS:
  --32              32-bit assembly
  
LDFLAGS:
  -m elf_i386       32-bit ELF
  -T link.ld        Use linker script
```

## 🧪 Testing Checklist

- [ ] Build succeeds without errors
- [ ] GDT initialization message appears
- [ ] IDT initialization message appears
- [ ] Timer initialization message appears
- [ ] No crashes or triple faults
- [ ] Can create processes with `ps`
- [ ] Can start scheduler with `start`
- [ ] Timer interrupts occur automatically

## 🆘 Common Issues

**Triple Fault on Boot:**
- Check GDT is loaded before enabling interrupts
- Verify IDT entries are properly initialized
- Ensure stack is properly aligned

**Timer Not Working:**
- Check PIC remapping (IRQ0 → 32)
- Verify EOI is sent to PIC
- Confirm timer handler is registered

**Scheduler Not Switching:**
- Ensure `scheduler_tick()` is called by timer
- Check time slice counter
- Verify context switching code

## 📖 Documentation Files

- `IMPLEMENTATION_SUMMARY.md` - What was implemented
- `INTERRUPT_IMPLEMENTATION.md` - How it works
- `ARCHITECTURE.md` - System design
- `QUICK_REFERENCE.md` - This cheat sheet

## ✅ Status

**Build**: ✅ Success  
**Boot**: ✅ Working  
**Interrupts**: ✅ Functional  
**Timer**: ✅ Running  
**Scheduler**: ✅ Preemptive  

---

**Version**: 1.0  
**Date**: January 4, 2026  
**Status**: Complete ✨
