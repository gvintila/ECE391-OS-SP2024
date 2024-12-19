/* Header file for IDT functions
*/
#ifndef IDT_H
#define IDT_H

#include "x86_desc.h"
#include "syscall.h"
#include "lib.h"

#define SYS_CALL_IDT_NUM   0x80
#define HALT_EXCEPTION_STATUS   256

#ifndef ASM

/* Flag used to halt on exceptions */
extern volatile int exception_found_flag;

/* Function to initialize IDT with entries */
extern void IDT_init();

/* ASM labels to link for SET_IDT_ENTRY for the IDT initialization */
extern void excep0();
extern void excep1();
extern void excep2();
extern void excep3();
extern void excep4();
extern void excep5();
extern void excep6();
extern void excep7();
extern void excep8();
extern void excep9();
extern void excep10();
extern void excep11();
extern void excep12();
extern void excep13();
extern void excep14();
extern void excep16();
extern void excep17();
extern void excep18();
extern void excep19();

/* declare our excepetion handlers*/
extern void excep_handler(uint32_t vector);
extern void excep_handler_error(uint32_t cr2, uint32_t vector, uint32_t edi, uint32_t esi, uint32_t ebp, uint32_t esp, uint32_t ebx, uint32_t edx, uint32_t ecx, uint32_t eax, uint32_t eflags, uint32_t error_code);


#endif // IDT_H

#endif // ASM
