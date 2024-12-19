/* IDT initialization for desired interrupts, exceptions, and system calls
*/

#include "idt.h"
#include "lib.h"
#include "x86_inter.h"
#include "pid.h"

/* local functions */
static void exceptions_init();
static void keyboard_idt_init();
static void rtc_idt_init();
static void pit_idt_init();
static void sys_call_init();

/* Exception string table */
char str_exceptions[20][50] =   {   "Divide Error Exception",
                                    "Debug Exception",
                                    "NMI Interrupt",
                                    "Breakpoint Exception",
                                    "Overflow Exception",
                                    "BOUND Range Exceeded Exception",
                                    "Invalid Opcode Exception",
                                    "Device Not Available Exception",
                                    "Double Fault Exception",
                                    "Coprocessor Segment Overrun",
                                    "Invalid TSS Exception",
                                    "Segment Not Present",
                                    "Stack Fault Exception",
                                    "General Protection Exception",
                                    "Page-Fault Exception",
                                    "RESERVED",
                                    "x87 FPU Floating-Point Error",
                                    "Alignment Check Exception",
                                    "Machine-Check Exception",
                                    "SIMD Floating-Point Exception",
                                };

/* IDT_init
 * Description: Initialize the IDT with the correct vector values
*            for the desired interrupts, exceptions, and system calls.
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 
*/
void IDT_init() {
    // Initiliaze the exceptions for IDT    
    exceptions_init();

    // Initialize the interrupts for IDT
    rtc_idt_init();

    keyboard_idt_init();

    pit_idt_init();

    // Initialize the system calls for IDT
    sys_call_init();

    // load the iDT
    lidt(idt_desc_ptr);
}

/* excep_handler
 * Description: Exception service routine to print the type of error
 * Inputs: vector - the vector number of the exception
 * Outputs: None
 * Side Effects: None
 

*/
void excep_handler(uint32_t vector) {
    cli();

    //clear();
    printf("%s raised! \n", str_exceptions[vector]);
    
    /* call halt to return control to shell when an exception fires */
    pcbs[curr_pid]->exception = 1;
    halt(0);

    /* freeze program */
    while(1) {
        continue;
    };

    sti();
}

/* excep_handler_error
 * Description: Exception service routine to print the type of error and error code
 * Inputs: vector - the vector number of the exception
 *         error_code - the error code of the exception
 *        registers - the registers at the time of the exception
 * Outputs: None
 * Side Effects: None
*/
void excep_handler_error(uint32_t cr2, uint32_t vector, uint32_t edi, uint32_t esi, uint32_t ebp, 
uint32_t esp, uint32_t ebx, uint32_t edx, uint32_t ecx, uint32_t eax, uint32_t eflags, uint32_t error_code) {
    cli();

    //clear();
    printf("%s raised! \n", str_exceptions[vector]);
    printf("edi: %x\n", edi);
    printf("esi: %x\n", esi);
    printf("ebp: %x\n", ebp);
    printf("esp: %x\n", esp);
    printf("ebx: %x\n", ebx);
    printf("edx: %x\n", edx);
    printf("ecx: %x\n", ecx);
    printf("eax: %x\n", eax);
    printf("eflags: %x\n", eflags);
    printf("Error code: %x\n", error_code);

    /* Page fault check */
    if (vector == 14) {
        printf("CR2 Reg: %d\n", cr2);
    }
    
    /* call halt to return control to shell when an exception fires */
    pcbs[curr_pid]->exception = 1;
    halt(0);

    /* freeze program */
    while(1) {
        continue;
    };

    sti();
}
/* exceptions_init
 * Description: Initialize the IDT entries for the exceptions by writing 
    *              the proper bits to the elements of the IDT
 * Inputs: None
 * Outputs: None
 * Side Effects: None
*/
static void exceptions_init() {
    int i;

    for (i = 0; i < 20; i++) {
        
        if (i != 15) {
            /* write proper bits except the address of handler */
            idt[i].seg_selector = KERNEL_CS;
            idt[i].reserved3 = 1;
            idt[i].reserved2 = 1;
            idt[i].reserved1 = 1;
            idt[i].size = 1;

            idt[i].present = 1;


        }
        
    }
    
    /* add proper offset  to each vector in IDT*/
    SET_IDT_ENTRY(idt[0], &excep0);
    SET_IDT_ENTRY(idt[1], &excep1);
    SET_IDT_ENTRY(idt[2], &excep2);
    SET_IDT_ENTRY(idt[3], &excep3);
    SET_IDT_ENTRY(idt[4], &excep4);
    SET_IDT_ENTRY(idt[5], &excep5);
    SET_IDT_ENTRY(idt[6], &excep6);
    SET_IDT_ENTRY(idt[7], &excep7);
    SET_IDT_ENTRY(idt[8], &excep8);
    SET_IDT_ENTRY(idt[9], &excep9);
    SET_IDT_ENTRY(idt[10], &excep10);
    SET_IDT_ENTRY(idt[11], &excep11);
    SET_IDT_ENTRY(idt[12], &excep12);
    SET_IDT_ENTRY(idt[13], &excep13);
    SET_IDT_ENTRY(idt[14], &excep14);
    /*skip vector 15*/
    SET_IDT_ENTRY(idt[16], &excep16);
    SET_IDT_ENTRY(idt[17], &excep17);
    SET_IDT_ENTRY(idt[18], &excep18);
    SET_IDT_ENTRY(idt[19], &excep19);
}

/* sys_call_init
 * Description: Initialize the IDT entry for the system call by writing 
    *              the proper bits to the elements of its IDT
 * Inputs: None
 * Outputs: None
 * Side Effects: Initializes system call.
*/
static void sys_call_init(){
    idt[SYS_CALL_IDT_NUM].seg_selector = KERNEL_CS;
    idt[SYS_CALL_IDT_NUM].present = 1;
    idt[SYS_CALL_IDT_NUM].size = 1;
    idt[SYS_CALL_IDT_NUM].reserved1 = 1;
    idt[SYS_CALL_IDT_NUM].reserved2 = 1;
    idt[SYS_CALL_IDT_NUM].reserved3 = 1;
    idt[SYS_CALL_IDT_NUM].dpl = 3;

    SET_IDT_ENTRY(idt[SYS_CALL_IDT_NUM], &sys_call_handler_link);
}
/* rtc_idt_init
 * Description: Initialize the IDT entry for the RTC by writing 
    *              the proper bits to the elements of its IDT
 * Inputs: None
 * Outputs: None
 * Side Effects: Initializes RTC.
*/
static void rtc_idt_init() {
    /* Initialize IDT entry corresponding to RTC */
    idt[RTC_IDT_NUM].seg_selector = KERNEL_CS;
    idt[RTC_IDT_NUM].present = 1;
    idt[RTC_IDT_NUM].size = 1;
    idt[RTC_IDT_NUM].reserved1 = 1;
    idt[RTC_IDT_NUM].reserved2 = 1;
    idt[RTC_IDT_NUM].reserved3 = 0;

    /* Set handler for RTC IDT entry by linking to rtc_handler() */
    SET_IDT_ENTRY(idt[RTC_IDT_NUM], &rtc_handler_link);
}

/* keyboard_idt_init
 * Description: Initialize the IDT entry for the keyboard by writing 
    *              the proper bits to the elements of its IDT
 * Inputs: None
 * Outputs: None
 * Side Effects: Initializes keyboard.
*/
static void keyboard_idt_init() {
    /* Initialize IDT entry corresponding to RTC */
    idt[KEYBOARD_IDT_NUM].seg_selector = KERNEL_CS;
    idt[KEYBOARD_IDT_NUM].present = 1;
    idt[KEYBOARD_IDT_NUM].size = 1;
    idt[KEYBOARD_IDT_NUM].reserved1 = 1;
    idt[KEYBOARD_IDT_NUM].reserved2 = 1;
    idt[KEYBOARD_IDT_NUM].reserved3 = 0;

    /* Set handler for keyboard IDT entry by linking to keyboard_handler() */
    SET_IDT_ENTRY(idt[KEYBOARD_IDT_NUM], &keyboard_handler_link);
}

/* pit_idt_init
 * Description: Initialize the IDT entry for the PIT by writing 
    *              the proper bits to the elements of its IDT
 * Inputs: None
 * Outputs: None
 * Side Effects: Initializes PIT.
*/
static void pit_idt_init() {
    /* Initialize IDT entry corresponding to PIT */
    idt[PIT_IDT_NUM].seg_selector = KERNEL_CS;
    idt[PIT_IDT_NUM].present = 1;
    idt[PIT_IDT_NUM].size = 1;
    idt[PIT_IDT_NUM].reserved1 = 1;
    idt[PIT_IDT_NUM].reserved2 = 1;
    idt[PIT_IDT_NUM].reserved3 = 0;

    /* Set handler for keyboard IDT entry by linking to pit_handler() */
    SET_IDT_ENTRY(idt[PIT_IDT_NUM], &pit_handler_link);
}
