/* x86_inter.h - Defines functions for x86 interrupt handling
 * vim:ts=4 noexpandtab
 */

#ifndef _X86_INTER_H
#define _X86_INTER_H

#include "x86_desc.h"
#include "i8259.h"
#include "types.h"
#include "kboard.h"
#include "rtc.h"

/* Interrupts information regarding RTC, keyboard, and PIT */
#define RTC_IRQ   8
#define RTC_IDT_NUM   0x28

#define KEYBOARD_IRQ   1
#define KEYBOARD_IDT_NUM   0x21

#define PIT_IRQ   0
#define PIT_IDT_NUM    0x20

/* Misc. used for demos */
//if key "i" is pressed, RTC interrupts are enabled and the screen flickers
#define RTC_ENABLE_KEYCODE  0x17 
//if key "x" is pressed, RTC interrupts are disabled and the screen stops flickering
#define RTC_DISABLE_KEYCODE  0x2D 
//if key "r" is pressed, RTC interrupt frequency is changed
#define RTC_CHANGE_FREQUENCY_KEYCODE  0x13 

#ifndef ASM

/* Initialize interrupt in IDT */
extern void interrupts_init();

/* RTC handler code */
extern void rtc_handler();

/* RTC link function to be called from x86_interrupts.S */
extern void rtc_handler_link();

/* Keyboard handler code */
extern void keyboard_handler();

/* Keyboard link function to be called from x86_interrupts.S */
extern void keyboard_handler_link();

/* PIT handler code */
extern void pit_handler();

/* PIT link function to be called from x86_interrupts.S */
extern void pit_handler_link();

/* Sys call link function to be called form x86_interrupts.S */
extern void sys_call_handler_link();

/* Sys call handler code */
extern void sys_call_handler();


/* helps test rtc for demo */
extern int rtc_test_flag;

#endif /* _x86_INTER_H */

#endif /* ASM */
