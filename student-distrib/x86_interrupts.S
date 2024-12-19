/* Constants used to define system calls */
.data 
     SYS_ERROR = 0
     SYS_HALT = 1
     SYS_EXECUTE = 2
     SYS_READ = 3
     SYS_WRITE = 4
     SYS_OPEN = 5
     SYS_CLOSE = 6
     SYS_GETARGS = 7
     SYS_VIDMAP = 8
     SYS_SETHANDLER = 9
     SYS_SIGRETURN  = 10
     MAX_SYS = 12
     MIN_SYS = 1
     ERROR = -1
     EXCEPTION = 256
/* Offsets for input arguments for syscalls */
    ARG1 = 8;
    ARG2 = 12;
    ARG3 = 16;

/* Macro to help define basic link between 
 * interrupt/exception handler and label */
#define interrupt_link(name, func)     \
.global name                    ;\
name:                           ;\
    pushal                      ;\
    pushfl                      ;\
    call func                   ;\
    popfl                       ;\
    popal                       ;\
    sti                         ;\
    iret                        ;\

/* Macro to link system call to handler if 
 * it's invoked from the kernel space separate from
 * a user space program */
#define syscall_link(name, number)      \
.GLOBL name                  ;\
name:                        ;\
    pushl	%ebx             ;\
	movl	$number,%eax     ;\
	movl	ARG1(%esp),%ebx  ;\
	movl	ARG2(%esp),%ecx  ;\
	movl	ARG3(%esp),%edx  ;\
	INT     $0x80            ;\
	popl	%ebx             ;\
	ret                      ;\

/* Syscall handler that jumps to corresponding function
 * depending on the the value in EAX */
#define syscall_jmp(name)             \
.GLOBL name                          ;\
name:                                ;\
	pushl   %ebx                     ;\
    pushl	%ecx                     ;\
    pushl	%edx                     ;\
    pushl	%edi                     ;\
    pushl	%esi                     ;\
    pushl	%ebp                     ;\
    pushl	%esp                     ;\
    pushfl                           ;\
    cmpl     $1, %eax                ;\
    jl      sys_error                ;\
    cmpl     $12, %eax               ;\
    jg      sys_error                ;\
    jmp     *syscall_table(,%eax,4)  ;\

/* RTC link */
interrupt_link(rtc_handler_link, rtc_handler);

/* Keyboard link */
interrupt_link(keyboard_handler_link, keyboard_handler)

/* PIT link */
interrupt_link(pit_handler_link, pit_handler)

/* Syscall link */
syscall_jmp(sys_call_handler_link)

/* Syscall wrappers */
syscall_link(halt, SYS_HALT)
syscall_link(execute, SYS_EXECUTE)
syscall_link(read, SYS_READ)
syscall_link(write, SYS_WRITE)
syscall_link(open, SYS_OPEN)
syscall_link(close, SYS_CLOSE)
syscall_link(getargs, SYS_GETARGS)
syscall_link(vidmap, SYS_VIDMAP)
syscall_link(set_handler, SYS_SETHANDLER)
syscall_link(sigreturn, SYS_SIGRETURN) 
                    
/* System call handlers
    * Each handler will push the appropriate arguments to the stack, 
    * call the corresponding system call function
    * and then jump to sys_finish to restore all registers and return
    * to the parent process
*/
sys_halt:
    pushl	%ebx 
    call    syscall_halt
    popl    %ebx
    jmp     sys_finish    

sys_execute:
    pushl	%ebx 
    call    syscall_execute
    cmp     $ERROR, %eax
    je      sys_execute_error
    xorl    %eax, %eax
    movl    %ebx, %eax
    popl    %ebx
    jmp     sys_finish

sys_execute_error:
    popl    %ebx
    jmp     sys_finish

sys_read:
    pushl	%edx 
    pushl   %ecx
    pushl   %ebx
    call    syscall_read
    popl    %ebx
    popl    %ecx
    popl    %edx
    jmp     sys_finish                   

sys_write:
    pushl	%edx 
    pushl   %ecx
    pushl   %ebx
    call    syscall_write
    popl    %ebx
    popl    %ecx
    popl    %edx
    jmp     sys_finish                 

sys_open:
    pushl	%ebx 
    call    syscall_open
    popl    %ebx
    jmp     sys_finish                  

sys_close:
    pushl	%ebx 
    call    syscall_close
    popl    %ebx
    jmp     sys_finish                  

sys_getargs:
    pushl	%ecx 
    pushl   %ebx
    call    syscall_getargs
    popl    %ebx
    popl    %ecx
    jmp     sys_finish                    

sys_vidmap:
    pushl	%ebx 
    call    syscall_vidmap
    popl    %ebx
    jmp     sys_finish

sys_set_handler:
    pushl	%ecx 
    pushl   %ebx
    call    syscall_set_handler
    popl    %ebx
    popl    %ecx
    jmp     sys_finish                    

sys_sigreturn:
    call    syscall_sigreturn
    jmp     sys_finish       

sys_malloc:
    pushl	%ebx 
    call    syscall_malloc
    popl    %ebx
    jmp     sys_finish       

sys_free:
    pushl	%ebx 
    call    syscall_free
    popl    %ebx
    jmp     sys_finish                      

/* Use this to return early if we encounter any invalid parameters before jumping */
sys_error:
    movl    $-1, %eax
    jmp     sys_finish                   

/* Use this to restore all registers after a system call and call iret to go back to parent process */
sys_finish:
    popfl
    popl	%esp                     
    popl	%ebp                     
    popl	%esi                     
    popl	%edi                     
    popl	%edx                     
    popl	%ecx                     
    popl	%ebx                     
    iret                     
    
/* Jump table to jump to handler for each system call */
syscall_table:
    .long sys_error, sys_halt, sys_execute, sys_read, sys_write, sys_open, sys_close, sys_getargs, sys_vidmap, sys_set_handler, sys_sigreturn, sys_malloc, sys_free

