/* context_switch.S - Context switching using iret */

.global switch_to_process
.global start_first_process

/* 
 * switch_to_process(process_t *old_proc, process_t *new_proc)
 */
switch_to_process:
    movl 4(%esp), %eax          /* old_proc */
    movl 8(%esp), %edx          /* new_proc */
    
    /* Save old process */
    testl %eax, %eax
    jz load_new
    
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %esi
    pushl %edi
    pushl %ebp
    pushfl
    
    movl %esp, 44(%eax)
    
load_new:
    /* Switch to new process stack */
    movl 44(%edx), %esp
    
    /* Restore new process registers */
    /* NOTE: We restore EFLAGS but ensure IF bit is set */
    popl %ecx               /* Pop EFLAGS into ECX instead of directly with popfl */
    orl $0x200, %ecx        /* Set IF bit */
    pushl %ecx
    popfl                   /* Now restore modified EFLAGS */
    
    popl %ebp
    popl %edi
    popl %esi
    popl %edx
    popl %ecx
    popl %ebx
    
    /* Get return address (process entry/resume point) */
    popl %eax
    
    /* Set up iret frame */
    pushfl
    orl $0x200, (%esp)      /* Ensure IF is set in the EFLAGS we'll use for iret */
    pushl $0x08             /* CS */
    pushl %eax              /* EIP */
    
    /* Jump to process via iret */
    iret

start_first_process:
    movl 4(%esp), %eax
    movl 44(%eax), %esp
    
    popfl
    popl %ebp
    popl %edi
    popl %esi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax           /* Entry point */
    
    /* Set up iret frame */
    pushl $0x202        /* EFLAGS with IF=1 */
    pushl $0x08         /* CS */
    pushl %eax          /* EIP */
    
    iret
