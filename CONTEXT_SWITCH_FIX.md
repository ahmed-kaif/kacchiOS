# Context Switch Bug Analysis

## The Problem

After switching from idle to worker1, NO further context switches occur because timer interrupts stop firing.

## Root Cause

When `switch_to_process()` is called during an interrupt:
1. We're on Process A's stack (with full interrupt frame)
2. We save Process A's registers
3. **We switch to Process B's stack** (which only has basic registers, no interrupt frame)
4. We restore Process B's registers
5. We `ret` - which pops from **Process B's stack** and jumps to Process B's entry point
6. **We never return to the interrupt handler to do `iret`**
7. **The CPU thinks we're still in an interrupt, so it blocks all further interrupts!**

## The Fix

Option 1 (Complex but correct): Set up process stacks to include full interrupt frames
- When switching TO a process, craft an interrupt frame on its stack
- Use `iret` to return to the process
- Requires significant changes to process initialization and context switch assembly

Option 2 (Simpler workaround): Don't switch stacks during interrupt
- Mark that a switch is needed
- Return from interrupt normally
- Switch in a controlled context (but this isn't truly preemptive)

Option 3 (What we'll do): Fix the assembly to properly handle the return
- Save/restore enough state to return through the interrupt handler
- Ensure the return path works even after stack switch

## Implementation

We need to ensure that after switching stacks, when we `ret` from `switch_to_process`,
we somehow get back to the interrupt handler path. This requires making the new
process's stack have a proper "resumption point".
