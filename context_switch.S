/* context_switch.S - Assembly code for context switching */

.global switch_to_process
.global start_first_process

/* 
 * void switch_to_process(process_t *old_proc, process_t *new_proc)
 * 
 * Saves the context of old_proc and restores the context of new_proc
 * 
 * Process structure offsets:
 *   stack_ptr: 44 bytes (uint32_t*)
 */
switch_to_process:
    /* Get parameters before modifying stack */
    movl 4(%esp), %eax          /* old_proc */
    movl 8(%esp), %edx          /* new_proc */
    
    /* Save old process context if old_proc is not NULL */
    testl %eax, %eax
    jz load_new_context
    
    /* Save return address first */
    movl (%esp), %ecx           /* Get return address */
    
    /* Save registers in order: EBX, ECX, EDX, ESI, EDI, EBP, EFLAGS, RET */
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %esi
    pushl %edi
    pushl %ebp
    pushfl
    pushl %ecx                  /* Push return address */
    
    /* Save ESP to old_proc->stack_ptr (offset 44) */
    movl %esp, 44(%eax)
    
load_new_context:
    /* Restore ESP from new_proc->stack_ptr (offset 44) */
    movl 44(%edx), %esp
    
    /* Pop everything in reverse order */
    popl %ecx                   /* Pop return address */
    popfl                       /* Restore flags */
    popl %ebp
    popl %edi
    popl %esi
    popl %edx
    popl %ecx
    popl %ebx
    
    /* Jump to return address (either resume_here or entry_point) */
    jmp *%ecx

/*
 * void start_first_process(process_t *proc)
 * 
 * Starts the first process (no context to save)
 */
start_first_process:
    movl 4(%esp), %eax          /* Load proc pointer */
    
    /* Load stack pointer from proc->stack_ptr (offset 44) */
    movl 44(%eax), %esp
    
    /* Stack has: EBX, ECX, EDX, ESI, EDI, EBP, EFLAGS, entry_point */
    /* Pop everything */
    popl %ecx                   /* Pop entry_point */
    popfl                       /* Restore EFLAGS */
    popl %ebp
    popl %edi
    popl %esi
    popl %edx
    popl %ecx
    popl %ebx
    
    /* Jump to entry point */
    push %ecx                   /* Use the entry point we popped */
    ret

