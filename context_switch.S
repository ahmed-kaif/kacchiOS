/* context_switch.S - Assembly code for context switching */

.global switch_to_process
.global start_first_process

/* 
 * void switch_to_process(process_t *old_proc, process_t *new_proc)
 * 
 * Saves the context of old_proc and restores the context of new_proc
 * 
 * Process structure offsets:
 *   esp: 44 bytes
 *   ebp: 48 bytes
 *   eip: 52 bytes
 *   eflags: 56 bytes
 */
switch_to_process:
    /* Save old process context if old_proc is not NULL */
    movl 4(%esp), %eax          /* Load old_proc pointer into eax */
    testl %eax, %eax            /* Check if old_proc is NULL */
    jz load_new_context         /* If NULL, skip saving */

    /* Save all general purpose registers */
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %esi
    pushl %edi
    pushl %ebp
    
    /* Save flags */
    pushfl
    
    /* Save current ESP to old_proc->esp (offset 44) */
    movl %esp, 44(%eax)
    
    /* Save EBP to old_proc->ebp (offset 48) */
    movl %ebp, 48(%eax)
    
load_new_context:
    /* Load new process pointer */
    movl 8(%esp), %eax          /* Load new_proc pointer into eax */
    
    /* Restore ESP from new_proc->esp (offset 44) */
    movl 44(%eax), %esp
    
    /* Restore EBP from new_proc->ebp (offset 48) */
    movl 48(%eax), %ebp
    
    /* Restore flags */
    popfl
    
    /* Restore all general purpose registers */
    popl %ebp
    popl %edi
    popl %esi
    popl %edx
    popl %ecx
    popl %ebx
    
    /* Return to new process */
    ret

/*
 * void start_first_process(process_t *proc)
 * 
 * Starts the first process (no context to save)
 * This sets up the stack and jumps to the process entry point
 */
start_first_process:
    movl 4(%esp), %eax          /* Load proc pointer */
    
    /* Load stack pointer from proc->esp (offset 44) */
    movl 44(%eax), %esp
    
    /* Load base pointer from proc->ebp (offset 48) */
    movl 48(%eax), %ebp
    
    /* Load entry point from proc->eip (offset 52) */
    movl 52(%eax), %ecx
    
    /* Clear all registers for clean start */
    xorl %eax, %eax
    xorl %ebx, %ebx
    xorl %edx, %edx
    xorl %esi, %esi
    xorl %edi, %edi
    
    /* Enable interrupts */
    sti
    
    /* Jump to process entry point */
    jmp *%ecx

