/* x86_inter.c - Functions for calling interrupt handling
 * vim:ts=4 noexpandtab
 */

#include "x86_inter.h"
#include "x86_desc.h"
#include "kboard.h"
#include "rtc.h"
#include "lib.h"
#include "terminal.h"
#include "scheduler.h"


int rtc_test_flag = 0;


/* void interrupts_init()
 * Inputs: void
 * Return Value: void
 * Function: Turns on all required interrupts for our OS.
 * Effects: writes to registers
 *  
 */
void interrupts_init() {
    /* Turn on RTC interrupts */
    // RTC interrupts will be turned on when a specific key is pressed

    /* Turn on keyboard interrupts */
    enable_kboard_interrupt();
   
}

/* void rtc_handler()
 * Inputs: void
 * Return Value: void
 * Function: Interrupt handler for RTC.
 * Effects: writes to registers
 *  
 */
void rtc_handler() {
    /* Virtualized RTC gives each terminal the illusion that it can set the RTC rate, even though
     * it will be set to a constant 512 Hz. This is due to the fact that there is only one RTC within
     * the OS but we want each process to be able to use it at different rates. We iterate through each 
     * terminal, and set their read flag to +1 to show how many RTC ticks have been processed. We also 
     * reset the counter and make sure to only increment it if that terminal has opened RTC
     */

    /* Iterate through each terminal and update RTC data if a process has opened RTC in that terminal */
    int i;
    for(i = 0;i < TERMINAL_COUNT;++i){
        if(terminal_rtc_data[i].rtc_rate != -1){
            terminal_rtc_data[i].rtc_counter++;
            /* If the terminal's virtualized RTC needs to be fired, update the read flag and reset counter */
            if(terminal_rtc_data[i].rtc_counter == terminal_rtc_data[i].rtc_rate){
                terminal_rtc_data[i].rtc_counter = 0;
                terminal_rtc_data[i].rtc_read_flag++;//increment the read flag counter for a terminal that should fire an RTC tick
            }
        }
    }

    /* Throw away contents of register C of RTC */
    outb(RTC_REGISTER_C, RTC_INDEX_PORT);
    inb(RTC_DATA_PORT);	

    /* Send EOI at end of RTC handler */
    send_eoi(RTC_IRQ);
}

/* void  keyboard_handler
 * Inputs: void
 * Return Value: void
 * Function: Interrupt Handler for keyboard, writes to keyboard buf and to the screen as well.
 *           This also handles different states for keyboard such as CAPSLOCK and SHIFT
 * 
 *  
 */
void keyboard_handler() {
    uint32_t flags;
    cli_and_save(flags);

    key_map output_key = {0};
    uint8_t c;
    uint8_t scancode;

    /* Switch to shown terminal screen */
    switch_screen(terminal_active, terminal_shown, 0);

    /* get our value from the port*/
    uint32_t data = inb(KB_DATA_PORT);
    
   

    /* get the proper scancode whether it is released or pressed*/
    scancode = (uint8_t)(data & (~(1 << 7)));
    
    /* check if the port is an ACK bit */
    if (data == KB_ACK) {
        kboard_ack = 1;
        switch_screen(terminal_shown, terminal_active, 0);
        send_eoi(KEYBOARD_IRQ);
        restore_flags(flags);
        return;
    }
    /* Update our state keys */
    update_state_keys(scancode, data);
   
    /* Check if our scancode match a letter or number key (any printable key) */
    check_keys(scancode, data, &output_key);
    
    /* check for CTRL+L*/
    if ((key_state[CTRL_IDX].state) && (output_key.unshifted == 'l') && (output_key.state)) {
        clear();
        switch_screen(terminal_shown, terminal_active, 0);
        restore_flags(flags);
        send_eoi(KEYBOARD_IRQ);
        return;
    }

    /* Check for up or down (viewing saved command history) */
    if (key_state[UP_IDX].state || key_state[DOWN_IDX].state) {
        if (key_state[UP_IDX].state) type_command(1);
        else if (key_state[DOWN_IDX].state) type_command(0);
        switch_screen(terminal_shown, terminal_active, 0);
        restore_flags(flags);
        send_eoi(KEYBOARD_IRQ);
        return;
    }

    /* check if we are terminal switching */
    if ((key_state[ALT_IDX].state)) {
        int new_term;
        /* iterate through the f keys to see if one was pressed*/
        for (new_term = 0; new_term < 3; new_term++) {
            if (key_state[new_term + F1_IDX].state) {
                /* switch our terminal */
                terminal_switch(new_term);
                break;
            }
        }  
    } 

    /* check if we found a printable character*/
    if (!output_key.scancode || !output_key.state) {
        switch_screen(terminal_shown, terminal_active, 0);
        restore_flags(flags);
        send_eoi(KEYBOARD_IRQ);
        return;
    }
   
    /* get the proper character by checking the state of our keyboard */
    c = check_states(&output_key);

    /* check if it is a backspace because we do not add that to buffer or print it */
    if (output_key.scancode == BACKSPACE) backspace_pressed();
    
    
    /* BEFORE MODIFYING OUR BUFFER WE CHECK IF WE HAVE OVERFLOW */
    /* if our buffer is full i.e. we are at index 126 (i = 127 is our \n) we dont do anything*/
    if(check_buffer_overflow(&output_key)) {
        switch_screen(terminal_shown, terminal_active, 0);
        restore_flags(flags);
        send_eoi(KEYBOARD_IRQ);
        return;
    }
    
    /* add our character to the buffer and output to screen*/
    add_to_buffer(c, &output_key);

    /* Switch back to active terminal screen */
    switch_screen(terminal_shown, terminal_active, 0);


    /* Send EOI signal at end of handler */
    restore_flags(flags);
    send_eoi(KEYBOARD_IRQ);
}

void pit_handler(){
    //printf("PIT handler entered!\n");
    send_eoi(PIT_IRQ);
    schedule();
}
