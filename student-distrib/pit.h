/* rtc.h - Defines functions used to initialize RTC interrupts
 * vim:ts=4 noexpandtab
 */

#ifndef _PIT_H
#define _PIT_H

#include "x86_inter.h"
#include "lib.h"
#include "scheduler.h"

/* Ports used for PIT */
#define PIT_CHANNEL_0           0x40         
#define PIT_CHANNEL_1           0x41         
#define PIT_CHANNEL_2           0x42   
#define PIT_COMMAND_REGISTER    0x43

/* PIT rate/frequency data */
#define PIT_RATE_CONSTANT       1193182

/* PIT divisor, ms = millisecond */
#define PIT_DIVISOR(ms)         ( (ms/1000) * PIT_RATE_CONSTANT )

/* maths:
 * 1/f = T
 * f = PIT_RATE / DIVISOR
 * DIVISOR / PIT_RATE = T
 * DIVISOR = PIT_RATE * T */

/* Fuck C */
#define TERMINAL_COUNT      3

#ifndef ASM

/* Initialize PIT interrupts */
extern void pit_init();

/* Enable IRQ line connected to PIT */
extern void enable_pit_interrupt();

#endif /* _PIT_H */

#endif /* ASM */
