.data
    //offsets to pull curr and to pointers off the stack
    CURR_PTR = 8
    TO_PTR = 12
    //offsets to store registers in saved_regs_t
    EFLAGS_IDX = 0x0
    EIP_IDX = 0x4
    ESP_IDX = 0x8
    EBP_IDX = 0xC
    EAX_IDX = 0x10
    EBX_IDX = 0x14
    ECX_IDX = 0x18
    EDX_IDX = 0x1C
    ESI_IDX = 0x20
    EDI_IDX = 0x24
    //constants to setup for iret
    USER_DS = 0x002B
    USER_CS = 0x0023
    USER_ESP = 0x08400000

.global exec_context_switch, halt_context_switch, save_ctx_regs
/* 
 * context_switch
 * 
 * This function is called to switch from the current process to the next process
 * 
 * Inputs:
 *  %eax - %edi: the registers to save
 *  %ebp: the base pointer
 *  8(%ebp): the current process pointer
 *  12(%ebp): the next process pointer
 * 
 * Outputs:
 *  %eax - %edi: the registers to restore
 *  %ebp: the base pointer
 *  %esp: the stack pointer
 *  8(%ebp): the current process pointer
 *  12(%ebp): the next process pointer
 * 
 * Side Effects:
 *  The current process is saved and the next process is restored
 
*/
exec_context_switch:
    //make new stack frame
    pushl   %ebp
    movl    %esp, %ebp

    //store curr_ctx* in ebx
    pushl   %ebx
    movl    CURR_PTR(%ebp), %ebx

    //store current registers in curr_ctx struct
    movl    %eax, EAX_IDX(%ebx)
    movl    %ecx, ECX_IDX(%ebx)
    movl    %edx, EDX_IDX(%ebx)
    movl    %esi, ESI_IDX(%ebx)
    movl    %edi, EDI_IDX(%ebx)

    //eflags is now updated at the beginning of scheduler and execute
    //push and pop eflags
    //pushfl
    //popl    EFLAGS_IDX(%ebx)

    //store ebx and store curr_ctx* in ecx
    popl    EBX_IDX(%ebx)
    movl    CURR_PTR(%ebp), %ecx

    //store the old ebp and the eip to our struct
    
    //store old ebp to reg eax and eip to edx
    movl    (%ebp), %eax
    movl    %eax, ESP_IDX(%ecx)
    movl    4(%ebp), %edx

    //store them to our struct
    movl    %eax, EBP_IDX(%ecx)
    movl    %edx, EIP_IDX(%ecx)
    
    //get the address for to_ctx
    movl    TO_PTR(%ebp), %ecx

    //enable interrupts for new eflags
    orl     $0x0200, EFLAGS_IDX(%ecx)

    //store to_tx registers into our own registers
    movl    EAX_IDX(%ecx), %eax
    movl    EBX_IDX(%ecx), %ebx
    movl    EDX_IDX(%ecx), %edx
    movl    ESI_IDX(%ecx), %esi
    movl    EDI_IDX(%ecx), %edi

    //push our flags and pop them
    //pushl   EFLAGS_IDX(%ecx)
    //popfl

    //Iret stuff
    //push USER_DS, ESP (0x8400000), EFLAG, USER_CS, EIP (entry_point)
    pushl   $USER_DS
    pushl   $USER_ESP
    movl    TO_PTR(%ebp), %ecx
    pushl   EFLAGS_IDX(%ecx)
    pushl   $USER_CS
    movl    TO_PTR(%ebp), %ecx
    pushl   EIP_IDX(%ecx)

    // finally write ecx with proper val
    movl    ECX_IDX(%ecx), %ecx


    // call iret for context switch
    iret

/* halt_context_switch
 *
 * This function is called to halt the current process and return to the parent.
 * Inputs: current registers to be stored in the current process struct
 */  
halt_context_switch:
    //make new stack frame
    pushl   %ebp
    movl    %esp, %ebp

    //store curr_ctx* in ebx
    pushl   %ebx
    movl    CURR_PTR(%ebp), %ebx

    //store current registers in curr_ctx struct
    movl    %eax, EAX_IDX(%ebx)
    movl    %ecx, ECX_IDX(%ebx)
    movl    %edx, EDX_IDX(%ebx)
    movl    %esi, ESI_IDX(%ebx)
    movl    %edi, EDI_IDX(%ebx)

    //push and pop eflags
    pushfl
    popl    EFLAGS_IDX(%ebx)

    //store ebx and store curr_ctx* in ecx
    popl    EBX_IDX(%ebx)
    movl    CURR_PTR(%ebp), %ecx

    //store the old ebp and the eip to our struct
    
    //store old ebp to reg eax and eip to edx
    movl    (%ebp), %eax
    movl    %eax, ESP_IDX(%ecx)
    movl    4(%ebp), %edx

    //store them to our struct
    movl    %eax, EBP_IDX(%ecx)
    movl    %edx, EIP_IDX(%ecx)
    
    //get the address for to_ctx
    movl    TO_PTR(%ebp), %ecx

    //store to_tx registers into our own registers
    movl    EAX_IDX(%ecx), %eax
    movl    EBX_IDX(%ecx), %ebx
    movl    EDX_IDX(%ecx), %edx
    movl    ESI_IDX(%ecx), %esi
    movl    EDI_IDX(%ecx), %edi

    //push our flags and pop them
    pushl   EFLAGS_IDX(%ecx)
    popfl

    //testing if we restore ebp and esp and do leave, ret
    movl    EBP_IDX(%ecx), %ebp
    movl    %ebp, %esp

    // over write our ecx value with the value for our to_tx register
    movl	ECX_IDX(%ecx), %ecx

    
    leave
    ret

/* save_ctx_regs
 *
 * This function is called to save the current process inside the scheduler for the base shell
 * Inputs: current registers to be stored in the current process struct
 */  
save_ctx_regs:
    //make new stack frame
    pushl   %ebp
    movl    %esp, %ebp

    //callee saved regs
    pushl   %ebx
    pushl   %esi
    pushl   %edi

    //store curr_ctx* in ebx
    pushl   %ebx
    movl    CURR_PTR(%ebp), %ebx

    //store current registers in curr_ctx struct
    movl    %eax, EAX_IDX(%ebx)
    movl    %ecx, ECX_IDX(%ebx)
    movl    %edx, EDX_IDX(%ebx)
    movl    %esi, ESI_IDX(%ebx)
    movl    %edi, EDI_IDX(%ebx)

    //push and pop eflags
    pushfl
    popl    EFLAGS_IDX(%ebx)

    //store ebx and store curr_ctx* in ecx
    popl    EBX_IDX(%ebx)
    movl    CURR_PTR(%ebp), %ecx

    //store the old ebp and the eip to our struct
    
    //store old ebp to reg eax and eip to edx
    movl    (%ebp), %eax
    movl    %eax, ESP_IDX(%ecx)
    movl    4(%ebp), %edx

    //store them to our struct
    movl    %eax, EBP_IDX(%ecx)
    movl    %edx, EIP_IDX(%ecx)

    //callee saved regs
    popl    %edi
    popl    %esi 
    popl    %ebx

    leave
    ret
