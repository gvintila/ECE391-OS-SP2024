/* scheduler.c - Functions for initializing scheduling
 * vim:ts=4 noexpandtab
 */

#include "scheduler.h"

/* Start kernel with terminal 0 shown and active (terminal_active will increment in schedule()) */
volatile int32_t terminal_shown = 0;
volatile int32_t terminal_active = TERMINAL_COUNT - 1;

/* All terminals start with no processes */
volatile int32_t base_processes[TERMINAL_COUNT] = {-1, -1, -1};
volatile int32_t active_processes[TERMINAL_COUNT] = {-1, -1, -1};

/* The big boy 
 *
 * Schedules next active terminal and sets up the context switch, vidmap, keyboard, rtc, etc. */
void schedule() {
    /* Set up variables for scheduler */
    uint32_t old_term = terminal_active;
    uint32_t curr_term = (terminal_active + 1) % TERMINAL_COUNT;
    terminal_active = curr_term;

    /* Set up vidmap */
    change_vidmap(curr_term);

    /* Set up vmem */
    switch_screen(old_term, curr_term, 0);

    /* Set up rtc */


    /* If curr_term is not already active, start it */
    if (base_processes[curr_term] == -1) {
        clear();
        /* Save ctx registers before context switching */
        save_ctx_regs(&(pcbs[curr_pid]->curr_regs));
        execute((const uint8_t*)"shell");
    }
    /* Curr_term has already started, context switch */
    else {
        /* Update pid */
        uint32_t from_pid = curr_pid;
        uint32_t to_pid = active_processes[curr_term];
        curr_pid = to_pid;
        
        /* Page to next process */
        page_user_program(to_pid);

        /* Set up vars for context switch */
        tss.esp0 = (uint32_t)get_pstack_loc(to_pid);

        halt_context_switch(&(pcbs[from_pid]->curr_regs), &(pcbs[to_pid]->curr_regs));

        printf("im a big boy!!");   /* This print should never be reached */
    }
}

/* terminal_switch 
 *
 * Inputs: curr_term - new terminal that is to be displayed on screen
 * 
 * Displays new terminal on screen specified by user on keyboard input */
void terminal_switch(uint32_t curr_term) {
    uint32_t old_term = terminal_shown;
    terminal_shown = curr_term;

    /* Only change vmem and screens if terminals are different */
    if (old_term != curr_term) {
        /* Copy screen vmem to old term vmem */
        copy_term_vmem(old_term, 0);
        /* Copy curr term vmem to screen vmem */
        copy_term_vmem(curr_term, 1);

        /* Switch to new screen and update cursor */
        switch_screen(old_term, curr_term, 1);

        /* ^^^ The screen will switch back to the active process at the end of the keyboard handler */

        /* Update vidmap for active terminal */
        change_vidmap(terminal_active);
    }
}
