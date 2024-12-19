/* scheduler.h - Functions for initializing scheduling
 * vim:ts=4 noexpandtab
 */

#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "paging.h"
#include "pid.h"

#define TERMINAL_COUNT  3

#ifndef ASM

/* Shows which terminal is currently active in scheduler (TA) */
extern volatile int32_t terminal_active;

/* Shows which terminal is currently showcasing on screen (TS) */
extern volatile int32_t terminal_shown;

/* Showcases which processes are base for each terminal (-1 if terminal has no base) */
extern volatile int32_t base_processes[TERMINAL_COUNT];

/* Showcases which processes are active in each terminal (-1 if terminal has no active process) */
extern volatile int32_t active_processes[TERMINAL_COUNT];

/* The big boy */
extern void schedule();

/* Displays new terminal on screen specified by user on keyboard input */
extern void terminal_switch(uint32_t curr_term);

#endif /* _PAGING_H */

#endif /* ASM */
