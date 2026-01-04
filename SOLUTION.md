# The Context Switch Problem - Root Cause and Solution

## Root Cause
`switch_to_process()` never returns when switching between processes during an interrupt!

Evidence:
- We see "[CTX] About to call switch_to_process"
- We NEVER see "[CTX] Returned from switch_to_process"
- After `movl 44(%edx), %esp` switches to new process stack
- `ret` instruction pops the return address from the NEW stack (which is the process entry point)
- Execution jumps to the process and never returns through the interrupt handler
- CPU remains in "interrupt mode" with interrupts blocked
- Timer never fires again → no more scheduling

## The Solution
The standard OS solution is to use **separate kernel/interrupt stacks**. Since we don't have that, we need to:

1. **Keep the interrupt stack** - don't switch ESP to the process's stack entirely
2. **Save/restore process state** to/from the PCB
3. **Return normally** through the interrupt handler so `iret` can execute

Alternatively (simpler hack):
- Make the new process's stack contain a COPY of the entire call chain
- So when we `ret`, we go through: context_switch → scheduler_schedule → scheduler_tick → timer_handler → irq_handler → irq_common_stub → iret

But option 1 is cleaner.
