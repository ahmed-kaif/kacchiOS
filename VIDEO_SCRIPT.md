# kacchiOS Video Demonstration Script

## **Video Duration: 10-15 minutes**

---

## **INTRODUCTION (1 minute)**

### **[Scene 1: Title Slide]**
**[Show on screen: "kacchiOS - A Minimal Baremetal Operating System"]**

**Script:**
"Hello everyone! Today I'm excited to present kacchiOS, a minimal baremetal operating system I developed from scratch. This is a 32-bit x86 operating system written in C and Assembly that implements core OS functionalities including process management, preemptive multitasking, and hardware interrupt handling. Let's dive in and see what makes this OS tick!"

---

## **PART 1: PROJECT OVERVIEW (1.5 minutes)**

### **[Scene 2: Show file structure in VS Code]**

**Script:**
"First, let me show you the project structure. kacchiOS consists of approximately 2000 lines of code organized into several key components:

- **Boot sector and kernel entry** - boot.S for initial setup
- **Hardware abstraction layer** - GDT, IDT, and interrupt handling
- **Memory management** - Fixed-size block allocator
- **Process management** - Process table and context switching
- **Scheduler** - Round-robin preemptive multitasking
- **Device drivers** - Serial port and PIT timer

The entire OS is bare metal, meaning it runs directly on hardware without any underlying OS. It's built using GCC for cross-compilation and runs on QEMU for testing."

**[Show: Quick scroll through main files: kernel.c, process.c, scheduler.c, gdt.c, idt.c]**

---

## **PART 2: SYSTEM ARCHITECTURE (2 minutes)**

### **[Scene 3: Open ARCHITECTURE.md - show diagrams]**

**Script:**
"Let me explain the system architecture using these diagrams I created.

**[Point to System Component Diagram]**
The OS has a layered architecture. At the bottom, we have the hardware abstraction layer consisting of:
- The GDT for memory segmentation
- The IDT for interrupt handling
- The PIT timer running at 100 Hz

Above that, we have the process manager and scheduler handling up to 32 concurrent processes with 100 millisecond time slices. At the top is the memory manager using fixed 1MB blocks.

**[Point to Interrupt Flow Diagram]**
Here's how interrupts flow through the system. When a timer IRQ fires, the CPU pushes the current state, our assembly stub saves registers, then the C handler processes the interrupt, calls the scheduler, and finally we restore registers and return via IRET. This is the foundation of preemptive multitasking.

**[Point to Scheduling Timeline]**
The scheduler uses round-robin algorithm. Each process gets a 100ms time slice - that's 10 timer ticks. When the time expires, the scheduler automatically switches to the next ready process."

---

## **PART 3: BOOT SEQUENCE (2 minutes)**

### **[Scene 4: Open kernel.c - show kmain function]**

**Script:**
"Now let's look at the actual code, starting with the boot sequence. Everything begins in the kmain function."

**[Scroll to kmain, lines 92-120]**

**[Show this code:]**
```c
void kmain(void)
{
  /* Initialize hardware */
  serial_init();
  
  /* Initialize GDT (Global Descriptor Table) */
  gdt_init();
  
  /* Initialize IDT (Interrupt Descriptor Table) */
  idt_init();
  
  /* Initialize PIT timer */
  timer_init();
  
  /* Initialize memory manager */
  memory_init();
  
  /* Initialize process manager */
  process_init();
  
  /* Initialize scheduler */
  scheduler_init();
```

**Script:**
"The boot sequence is carefully ordered:

1. **Serial initialization** - This sets up our debug output
2. **GDT initialization** - Sets up memory segmentation for protected mode
3. **IDT initialization** - Configures interrupt handlers and remaps the PIC
4. **Timer initialization** - Starts the PIT at 100 Hz for preemptive scheduling
5. **Memory manager** - Sets up our heap allocator
6. **Process manager** - Initializes the process table
7. **Scheduler** - Configures round-robin scheduling

Each component builds on the previous ones. For example, we need the IDT before starting the timer because timer interrupts require interrupt handling to be set up."

---

## **PART 4: GDT - MEMORY SEGMENTATION (1.5 minutes)**

### **[Scene 5: Open gdt.c]**

**Script:**
"Let's dive into the Global Descriptor Table. The GDT is crucial for x86 protected mode."

**[Show gdt_init function, lines 15-40]**

**[Highlight this code:]**
```c
void gdt_init(void) {
    /* Setup GDT entries:
     * 0: Null descriptor (required by x86)
     * 1: Kernel code segment (0x08)
     * 2: Kernel data segment (0x10)
     * 3: User code segment (0x18)
     * 4: User data segment (0x20)
     * 5: TSS (Task State Segment) (0x28)
     */
    gdt_set_gate(0, 0, 0, 0, 0);                    
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);    
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);    
```

**Script:**
"We configure 6 segment descriptors. The first is always null as required by x86 architecture. Then we have kernel code and data segments covering the full 4GB address space. We also reserve segments for future user-mode processes. The flags 0x9A and 0x92 define these as executable code and writable data respectively. The 0xCF granularity flag enables 4KB page granularity and 32-bit protected mode.

The TSS - Task State Segment - is particularly important for interrupt handling, as it tells the CPU where to find the kernel stack when switching from user mode."

---

## **PART 5: IDT AND INTERRUPT HANDLING (2 minutes)**

### **[Scene 6: Open idt.c]**

**Script:**
"Next, the Interrupt Descriptor Table. This is how we handle hardware interrupts and CPU exceptions."

**[Show pic_remap function, lines 54-72]**

**[Highlight this code:]**
```c
static void pic_remap(void) {
    /* ICW2 - Set vector offsets */
    outb(0x21, 0x20);  /* Master PIC starts at IRQ 32 */
    outb(0xA1, 0x28);  /* Slave PIC starts at IRQ 40 */
```

**Script:**
"One critical step is PIC remapping. By default, the PIC uses interrupt vectors 0-15, which conflict with CPU exception vectors. We remap the master PIC to start at 32 and the slave at 40. This separates hardware interrupts from CPU exceptions.

**[Scroll to idt_init, lines 75-95]**

Now we set up all 256 interrupt vectors. The first 32 are CPU exceptions like divide-by-zero, page faults, and general protection faults. Vectors 32-47 are our remapped hardware interrupts. We also reserve vector 128 for system calls, though that's a future enhancement."

**[Show exception_messages array, lines 19-50]**

"I've defined descriptive messages for each CPU exception, which is invaluable for debugging when something goes wrong."
**[Scroll to idt_set_gate function]**

**[Highlight:]**
```c
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].flags = flags;
}
```

**Script:**
"Here's a critical detail - we use the flags parameter as-is. The 0x8E value we pass contains the present bit and privilege level for ring 0. This was actually a bug I had to fix - originally I was ORing these flags with 0x60, which forced all interrupts to ring 3 privilege level. That prevented hardware interrupts from firing in kernel mode!"
---

## **PART 6: TIMER AND PREEMPTIVE SCHEDULING (2 minutes)**

### **[Scene 7: Open timer.c]**

**Script:**
"The PIT timer is what enables preemptive multitasking. Let me show you how it works."

**[Show timer_init function, lines 52-75]**

**[Highlight:]**
```c
void timer_init(void) {
    /* Calculate divisor for desired frequency */
    uint32_t divisor = PIT_FREQUENCY / TIMER_FREQ;
    
    /* Install timer handler on IRQ0 */
    irq_install_handler(0, timer_handler);
    
    outb(0x43, 0x36);  /* Configure PIT */
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
```

**Script:**
"The PIT oscillates at 1.193 MHz. We can configure it to different frequencies - in this implementation, I'm using a configurable frequency for responsive scheduling. The magic number 0x36 configures channel 0 in square wave mode, with lobyte/hibyte access for setting the divisor."

**[Show timer_handler function, lines 17-47]**

**[Highlight:]**
```c
static void timer_handler(struct interrupt_registers *regs) {
    timer_ticks++;
    
    /* Call scheduler to potentially switch processes */
    scheduler_tick();
}
```

**Script:**
"Every timer tick increments our counter and calls the scheduler. The scheduler tracks how many ticks each process has used. After 10 ticks, it forces a context switch to the next process. This is preemptive multitasking in action! The scheduler is independent of the exact timer frequency, so we can tune performance by adjusting the timer rate."

---

## **PART 7: PROCESS MANAGEMENT (2 minutes)**

### **[Scene 8: Open process.c]**

**Script:**
"Now let's see how processes are managed."

**[Show process_create function, lines 60-120]**

**[Highlight key parts:]**
```c
pid_t process_create(const char *name, void (*entry_point)(void), 
                     uint32_t priority)
{
    /* Allocate 4KB stack for process */
    proc->stack_base = (uint32_t)kmalloc(PROC_STACK_SIZE);
    proc->stack_size = PROC_STACK_SIZE;
    
    /* Setup initial stack frame for context switching */
    setup_process_stack(proc, entry_point);
```

**Script:**
"When creating a process, we allocate a 4KB stack from our memory manager and set up an initial stack frame. This frame contains the entry point address and initial register values. The stack is carefully crafted so that when we perform our first context switch, it looks like we're returning from an interrupt."

**[Show process structure definition if in process.h]**

**Script:**
"Each process has a PCB - Process Control Block - containing:
- Process ID and name
- Current state: ready, running, blocked, or free
- Priority level
- Stack pointer and base
- Execution time tracking
- Message passing fields for IPC

We maintain a process table with up to 32 processes."

---

## **PART 8: CONTEXT SWITCHING (2 minutes)**

### **[Scene 9: Open context_switch.S]**

**Script:**
"Context switching is where the magic happens. This is pure assembly because we need precise control over CPU registers."

**[Show switch_to_process function, lines 6-55]**

**[Highlight:]**
```asm
switch_to_process:
    movl 4(%esp), %eax          /* old_proc */
    movl 8(%esp), %edx          /* new_proc */
    
    /* Save old process */
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %esi
    pushl %edi
    pushl %ebp
    pushfl
    
    movl %esp, 44(%eax)         /* Save stack pointer */
```

**Script:**
"First, we save the current process state. We push all general-purpose registers and flags onto the stack. The stack pointer is then saved into the old process's PCB at offset 44.

**[Highlight load_new section:]**
```asm
load_new:
    movl 44(%edx), %esp         /* Load new stack */
    
    popl %ecx                   /* Pop EFLAGS */
    orl $0x200, %ecx            /* Set interrupt flag */
    pushl %ecx
    popfl
```

**Script:**
"Then we switch to the new process's stack and restore all its registers. Notice this critical line - we ensure the interrupt flag is set so timer interrupts continue firing. Without this, we'd lose preemption after the first context switch!

**[Highlight iret section:]**
```asm
    pushfl
    pushl $0x08             /* CS */
    pushl %eax              /* EIP */
    iret
```

**Script:**
"Finally, we use IRET - interrupt return - to jump to the new process. We push EFLAGS, code segment, and instruction pointer, creating a fake interrupt frame. IRET pops these and jumps to the process. This is elegant because it works identically whether we're starting a new process or resuming an existing one."

---

## **PART 9: SCHEDULER IMPLEMENTATION (1.5 minutes)**

### **[Scene 10: Open scheduler.c]**

**Script:**
"The scheduler implements round-robin algorithm with time-slicing."

**[Show scheduler_tick function, lines 200-250]**

**[Highlight:]**
```c
void scheduler_tick(void)
{
    process_t *current = process_current();
    
    if (current != NULL)
    {
        current->time_slice++;
        current->total_time++;
        
        /* Check if time quantum expired */
        if (current->time_slice >= time_quantum)
        {
            scheduler_switch();
        }
    }
}
```

**Script:**
"Every timer tick, we increment the current process's time slice. When it reaches 10 ticks - our time quantum - we force a context switch.

**[Show find_next_process function, lines 15-55]**

**[Highlight:]**
```c
static process_t *find_next_process(void)
{
    /* Search for next ready process (round-robin) */
    for (i = 0; i < MAX_PROCESSES; i++)
    {
        int idx = (start_idx + i) % MAX_PROCESSES;
        process_t *proc = process_get(idx);
        
        if (proc != NULL && proc->state == PROC_READY)
        {
            return proc;
        }
    }
```

**Script:**
"The round-robin algorithm is simple but effective. Starting from the current process, we loop through the process table looking for the next ready process. The modulo operation wraps around, ensuring fair scheduling. Every process gets an equal chance to run."

---

## **PART 10: LIVE DEMONSTRATION (3 minutes)**

### **[Scene 11: Terminal - Build and Run]**

**Script:**
"Now for the exciting part - let's see it in action!"

**[Type and run:]**
```bash
make clean && make
```

**Script:**
"First, we compile the OS. The Makefile builds all our C files and assembly files, then links them according to our custom linker script."

**[Wait for compilation to finish]**

**[Type and run:]**
```bash
make run
```

**Script:**
"Now let's boot it in QEMU. Watch the serial output..."

**[Show boot sequence output:]**
```
GDT initialized with 6 descriptors.
IDT initialized with 256 descriptors.
PIC remapped (IRQs 32-47).
PIT timer initialized.
Frequency: 100 Hz (10 ms per tick)
Memory manager initialized (Fixed-size blocks).
Process manager initialized.
Scheduler initialized (Round-Robin).
Time quantum: 10 ticks
```

**Script:**
"Perfect! Each component initializes successfully. The timer frequency is configurable - you'll see the actual frequency displayed in the output.

**[Show process creation:]**
```
Creating demo processes...
Process created: PID=0, Name='idle', Priority=1
Process created: PID=1, Name='worker1', Priority=5
Process created: PID=2, Name='counter', Priority=5
```

**Script:**
"We've created three demo processes: an idle process, a worker, and a counter. Let's start the scheduler and watch them run.

**[Show scheduler output:]**
```
[SCHED] Starting scheduler...
Scheduler started.
[SCHED] First process: idle (PID 0)
[Idle] Interrupts enabled via STI
[Idle] Waiting for interrupt...
[TIMER] Timer interrupt fired!
[Idle] Running...
[Counter] Count: 0
[Worker] Work unit: 0
[Idle] Running...
[Counter] Count: 1
```

**Script:**
"Excellent! Watch how the output interleaves - each process runs for its time slice, then the scheduler switches to the next. The idle process runs, then counter, then worker, and it cycles back. This is preemptive multitasking working perfectly!

The processes don't have to yield manually - the scheduler preempts them automatically based on timer interrupts. This is a fundamental OS feature that most modern operating systems use. Getting this working required careful attention to interrupt handling, privilege levels, and the PIC configuration."

---

## **PART 11: MEMORY MANAGEMENT (1 minute)**

### **[Scene 12: Back to VS Code - open memory.c]**

**Script:**
"Let me quickly show you the memory management system."

**[Show kmalloc function]**

**[Highlight:]**
```c
void *kmalloc(size_t size)
{
    /* Simple first-fit allocation */
    for (i = 0; i < TOTAL_BLOCKS; i++)
    {
        if (!memory_bitmap[i])
        {
            memory_bitmap[i] = 1;
            blocks_used++;
            return (void *)(memory_pool + (i * BLOCK_SIZE));
        }
    }
```

**Script:**
"We use a simple fixed-size block allocator. Each block is 1MB, and we have 32 blocks available. A bitmap tracks which blocks are allocated. This is simple but effective for a minimal OS. It's sufficient for process stacks and kernel data structures.

For a production OS, you'd want a more sophisticated allocator with variable-sized blocks, but for learning purposes, this demonstrates the core concepts clearly."

---

## **PART 12: CONCLUSION AND KEY TAKEAWAYS (1.5 minutes)**

### **[Scene 13: Back to terminal showing running OS]**

**Script:**
"So there you have it - kacchiOS, a fully functional minimal operating system!

Let me summarize the key achievements:

**1. Bare Metal Operation** - This OS boots directly on x86 hardware with no underlying OS. Everything from interrupt handling to process scheduling is implemented from scratch.

**2. Protected Mode** - We properly configure the GDT and enter 32-bit protected mode with memory segmentation.

**3. Hardware Interrupts** - The IDT handles both CPU exceptions and hardware interrupts. The PIC is properly remapped and configured.

**4. Preemptive Multitasking** - The PIT timer drives automatic context switching. Processes run without needing to cooperatively yield.

**5. Process Management** - Up to 32 processes with individual stacks, priorities, and state tracking.

**6. Memory Management** - A working heap allocator using fixed-size blocks.

**7. Context Switching** - Assembly-level register save/restore using IRET for elegant process switching.

The total project is about 2000 lines of C and Assembly code, demonstrating that you don't need millions of lines to create a working OS. It's all about understanding the fundamentals.

This project taught me invaluable lessons about:
- How hardware and software interact at the lowest level
- The importance of proper interrupt handling and PIC configuration
- The subtleties of privilege levels and interrupt gates
- The complexity of multitasking and scheduling
- Low-level debugging techniques and methodical troubleshooting
- The elegance of assembly when used appropriately
- The value of understanding x86 architecture documentation

**[Show the running processes one more time]**

While kacchiOS is minimal, it contains all the essential building blocks of a real operating system. From here, you could add:
- Virtual memory with paging
- A filesystem
- User-mode processes with system calls
- Device drivers for keyboard, display, network
- Inter-process communication
- And much more!

Thank you for watching this demonstration of kacchiOS! If you have questions or want to discuss the implementation details, feel free to ask. The complete source code is available, and I encourage you to explore it and maybe even build your own OS!

Happy coding!"

---

## **BONUS: Q&A SECTION (Optional - if time permits)**

### **Potential Questions to Address:**

**Q: "Why 32-bit instead of 64-bit?"**
A: "32-bit x86 is simpler to start with. It has fewer special cases, simpler paging structures, and more straightforward interrupt handling. It's perfect for learning. 64-bit adds complexity that can obscure the fundamental concepts."

**Q: "Could this run on real hardware?"**
A: "Absolutely! You could write this to a USB drive and boot a real x86 machine. However, you'd need to add a proper bootloader like GRUB and handle more hardware variations. QEMU is safer for development."

**Q: "What was the hardest part to implement?"**
A: "Interrupt handling was definitely the trickiest. I discovered a subtle bug where the IDT gate setup was forcing all interrupts to ring 3 privilege level, which prevented hardware interrupts from firing in kernel mode. I also had to ensure EOI (End of Interrupt) signals were sent properly to the PIC. Getting the stack frame layout correct for context switching and ensuring interrupts remain enabled took considerable debugging!"

**Q: "What resources did you use to learn this?"**
A: "The OSDev wiki is invaluable. Intel's processor manuals are the authoritative reference. I also studied existing OS code like Linux and MINIX to understand real-world implementations."

---

## **VIDEO PRODUCTION NOTES:**

### **Visual Elements to Include:**
- **Code highlighting** - Use VS Code with a clear theme
- **Diagrams** - Show the architecture diagrams from ARCHITECTURE.md
- **Split screen** - Code on one side, output on the other during demonstration
- **Zoom in** on important code sections
- **Text overlays** for key concepts
- **Animated arrows** to show flow of execution (optional)

### **Audio Tips:**
- Speak clearly and at moderate pace
- Pause between sections for emphasis
- Use enthusiasm when showing working demo
- Vary your tone to maintain interest

### **Pacing:**
- Don't rush through code explanations
- Let the demo run for several seconds so viewers can see the output
- Pause after important concepts to let them sink in

### **Editing:**
- Add chapter markers for each section
- Include timestamps in description
- Consider adding background music during intro/outro (low volume)
- Cut any long compilation waits

---

## **SCRIPT TIMING BREAKDOWN:**

1. Introduction: 1 min
2. Project Overview: 1.5 min
3. System Architecture: 2 min
4. Boot Sequence: 2 min
5. GDT: 1.5 min
6. IDT: 2 min
7. Timer: 2 min
8. Process Management: 2 min
9. Context Switching: 2 min
10. Scheduler: 1.5 min
11. Live Demo: 3 min
12. Memory: 1 min
13. Conclusion: 1.5 min

**Total: ~22 minutes** (can be edited down to 15 minutes by reducing code explanations)

---

## **THUMBNAIL SUGGESTIONS:**
- Screenshot of terminal showing processes running
- Text: "Building an OS from Scratch"
- Your face (if you're comfortable)
- OS architecture diagram in background

**Good luck with your video! 🚀**
