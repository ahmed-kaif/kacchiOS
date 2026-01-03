# 🍚 kacchiOS

A minimal, educational baremetal operating system designed for teaching OS fundamentals.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![Platform](https://img.shields.io/badge/platform-x86-lightgrey.svg)]()

## 📖 Overview

kacchiOS is a simple, bare-metal operating system built from scratch for educational purposes. It provides a clean foundation for students to learn operating system concepts by implementing core components themselves.

### Current Features

- ✅ **Multiboot-compliant bootloader** - Boots via GRUB/QEMU
- ✅ **Serial I/O driver** (COM1) - Communication via serial port
- ✅ **Memory Manager** - Simple bump allocator for dynamic memory
- ✅ **Process Manager** - Process creation, termination, and lifecycle management
- ✅ **Round-Robin Scheduler** - Time-sliced process scheduling
- ✅ **Interactive shell** - Command-line interface with process management commands
- ✅ **Basic string utilities** - Essential string operations
- ✅ **Clean, documented code** - Easy to understand and extend

### Future Extensions (Student Assignments)

Students can extend kacchiOS by implementing:
- 📝 **Complete Context Switching** - Full assembly-level register save/restore
- 📝 **Timer Interrupt Integration** - Hardware timer for automatic preemption
- 📝 **Priority Scheduling** - Enhance scheduler to respect process priorities
- 📝 **Process Synchronization** - Mutexes, semaphores, condition variables
- 📝 **Inter-Process Communication** - Message passing or shared memory
- 📝 **System Calls** - System call interface for user processes

## 🚀 Quick Start

### Prerequisites

```bash
# On Ubuntu/Debian
sudo apt-get install build-essential qemu-system-x86 gcc-multilib

# On Arch Linux
sudo pacman -S base-devel qemu gcc-multilib

# On macOS
brew install qemu i686-elf-gcc
```

### Build and Run

```bash
# Clone the repository
git clone https://github.com/yourusername/kacchiOS.git
cd kacchiOS

# Build the OS
make clean
make

# Run in QEMU
make run
```

You should see:
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

Available commands:
  ps       - List all processes
  stats    - Show scheduler statistics
  help     - Show this help message
  exit     - Halt the system

kacchiOS> 
```

### Available Commands

- `ps` - List all processes with their states and priorities
- `stats` - Show scheduler statistics and current process info
- `help` - Display available commands
- `exit` - Halt the system

Type a command and press Enter!

## 📁 Project Structure

```
kacchiOS/
├── boot.S                      # Bootloader entry point (Assembly)
├── kernel.c                    # Main kernel with process demo
├── serial.c                    # Serial port driver (COM1)
├── serial.h                    # Serial driver interface
├── string.c                    # String utility functions
├── string.h                    # String utility interface
├── memory.c                    # Memory manager (bump allocator)
├── memory.h                    # Memory manager interface
├── process.c                   # Process manager implementation
├── process.h                   # Process manager interface
├── scheduler.c                 # Round-robin scheduler implementation
├── scheduler.h                 # Scheduler interface
├── types.h                     # Basic type definitions
├── io.h                        # I/O port operations
├── link.ld                     # Linker script
├── Makefile                    # Build system
├── README.md                   # This file
└── PROCESS_SCHEDULER_GUIDE.md  # Detailed implementation guide
```

## 🛠️ Build System

### Makefile Targets

| Command | Description |
|---------|-------------|
| `make` or `make all` | Build kernel.elf |
| `make run` | Run in QEMU (serial output only) |
| `make run-vga` | Run in QEMU (with VGA window) |
| `make debug` | Run in debug mode (GDB ready) |
| `make clean` | Remove build artifacts |

## 📚 Learning Resources

### Implementation Guide

See [PROCESS_SCHEDULER_GUIDE.md](PROCESS_SCHEDULER_GUIDE.md) for detailed documentation on:
- Process Control Block (PCB) structure
- Process states and lifecycle
- Round-robin scheduling algorithm
- Context switching framework
- Usage examples and testing
- Future enhancement ideas

### Recommended Reading

- [XINU OS](https://xinu.cs.purdue.edu/) - Educational OS similar to kacchiOS
- [OSDev Wiki](https://wiki.osdev.org/) - Comprehensive OS development guide
- [The Little OS Book](https://littleosbook.github.io/) - Practical OS development
- [Operating Systems: Three Easy Pieces](https://pages.cs.wisc.edu/~remzi/OSTEP/) - OS concepts textbook

### Related Topics

- x86 Assembly Language
- Process Management
- Memory Management
- Process Scheduling Algorithms
- Context Switching
- System Calls
- Interrupt Handling

## 🤝 Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.

### Guidelines

1. Keep code simple and educational
2. Add comments explaining complex concepts
3. Follow existing code style
4. Test changes in QEMU before submitting

## 📄 License

This project is licensed under the MIT License.

## 👨‍🏫 About

kacchiOS was created as an educational tool for teaching operating system concepts. It provides a minimal, working foundation that students can extend to learn core OS principles through hands-on implementation.

## 🙏 Acknowledgments

- Inspired by XINU OS
- Built with guidance from OSDev community
- Thanks to all students who have contributed
