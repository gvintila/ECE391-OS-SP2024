/*
    kboard.c - functions related to intializing the keyboard

*/
#include "kboard.h"
#include "terminal.h"


/* ALL keys that can be printed will be mapped to an ASCII character */
/* Note that bit 7 of the scan code decides if it is a key pressed (0) or key is released(1) */
/* keys that can be printed out if we dont hold shift*/


key_map key_letters[NUM_LETTERS] = {

    {0x10, 'q', 'Q', 0},
    {0x11, 'w', 'W', 0},
    {0x12, 'e', 'E', 0},
    {0x13, 'r', 'R', 0},
    {0x14, 't', 'T', 0},
    {0x15, 'y', 'Y', 0},
    {0x16, 'u', 'U', 0},
    {0x17, 'i', 'I', 0}, 
    {0x18, 'o', 'O', 0},
    {0x19, 'p', 'P', 0},
    {0x1E, 'a', 'A', 0},
    {0x1F, 's', 'S', 0},
    {0x20, 'd', 'D', 0},
    {0x21, 'f', 'F', 0},
    {0x22, 'g', 'G', 0},
    {0x23, 'h', 'H', 0},
    {0x24, 'j', 'J', 0},
    {0x25, 'k', 'K', 0},
    {0x26, 'l', 'L', 0},
    {0x2C, 'z', 'Z', 0}, 
    {0x2D, 'x', 'X', 0}, 
    {0x2E, 'c', 'C', 0}, 
    {0x2F, 'v', 'V', 0}, 
    {0x30, 'b', 'B', 0}, 
    {0x31, 'n', 'N', 0}, 
    {0x32, 'm', 'M', 0},
};

key_map key_symbols[NUM_SYMBOLS] = {
    {0x02, '1', '!', 0},
    {0x03, '2', '@', 0},
    {0x04, '3', '#', 0},
    {0x05, '4', '$', 0},
    {0x06, '5', '%', 0},
    {0x07, '6', '^', 0},
    {0x08, '7', '&', 0},
    {0x09, '8', '*', 0},
    {0x0A, '9', '(', 0},
    {0x0B, '0', ')', 0},

    {0x0C, '-', '_', 0},
    {0x0D, '=', '+', 0},
    {0x1A, '[', '{', 0},
    {0x1B, ']', '}', 0},
    {0x27, ';', ':', 0},
    {0x28, '\'', '\"', 0},
    {0x29, '`', '~', 0},
    {0x2B, '\\', '|', 0},
    {0x33, ',', '<', 0},
    {0x34, '.', '>', 0},
    {0x35, '/', '?', 0},


    /* SPACE */
    {0x39, ' ', ' ', 0},
    /* ENTER */
    {0x1C, '\n', '\n', 0},
    /*back space */
    {0x0E, '\b', '\b', 0},
    /* TAB */
    {0x0F, '\t', '\t', 0}
};

/* Define our special state keys here */
key_state_map key_state[NUM_STATE_KEYS] = {
    /* left shift*/
    {0x2A, 0},
    /* right shift*/
    {0x36, 0},
    /* left CTRL */
    {0x1D, 0},
    /* caps lock */
    {0x3A, 0},
    /* alt */
    {0x38, 0},
    /* F1 - F3 KEYS*/
    {0x3B, 0},
    {0x3C, 0},
    {0x3D, 0},
    /* Up */
    {0x48, 0},
    /* Down */
    {0x50, 0}
};

/* Set ack flag to 0 before commands*/
volatile int kboard_ack = 0;

/* void  enable_kboard_interrupt()
 * Inputs: void
 * Return Value: void
 * Function: enables the irq line for the keyboard, writes to the PIC
 * 
 *  
 */
void enable_kboard_interrupt() {
    /* Enable IRQ corresponding to keyboard interrupts */
    enable_irq(KEYBOARD_IRQ);
}

/* void kboard_init()
 * Inputs: void
 * Return Value: void
 * Function: handles our initialization of the keyboard by writing the proper scan code set we want to use
 * 
 *  
 */
void kboard_init() {
    /* tell keyboard we want to set its scan code*/
    outb(KB_SCANCODE, KB_DATA_PORT);

    /* wait for acknowledge from keyboard*/
    wait_ack();

    /* tell keyboard we want its scan code to be 1 and wait for ack*/
    outb(SCAN_MODE, KB_DATA_PORT);

    /* wait for acknowledge from keyboard*/
    wait_ack();
}

/* void wait_ack()
 * Inputs: void
 * Return Value: void
 * Function: waits for keyboard acknowledge and then sets the ack flag back down to 0, mainly used to wait for keyboard ack after
 * sending a command byte
 * 
 *  
 */
void wait_ack() {
    while(!kboard_ack) {
        continue;
    }
    kboard_ack = 0;
}

/* void update_state_keys()
 * Inputs: unit8_t scancode, uint8_t data
 * Return Value: 1 if found a key, 0 if none
 * Function: finds if a state key was clicked and adjusts its state correctly
 * 
 *  
 */
void update_state_keys(uint8_t scancode, uint8_t data) {
    int i;
    /* check if it is a state key*/
    for (i = 0; i < NUM_STATE_KEYS; i++) {
        if (key_state[i].scancode == scancode) {
            /* if it is caps lock we only toggle when we press*/
            if (scancode == CAPS_LOCK) {
                if ((!(data & (1 << 7)))) key_state[i].state ^= 1;
                break; 
            }
            /* check to see if the data is a pressed or release key */
            if (data & (1 << 7)) {
                key_state[i].state = 0;
            } else {
                key_state[i].state = 1;    
            }
            break;
        }
    }
}

/* void check_keys()
 * Inputs: (uint8_t scancode, uint8_t data, key_map *output_key)
            These inputs are the scancodes that were pressed or released and
            a pointer to the key_map struct with the info of the key.
 * Return Value: void
 * Function: goes through all letters and numbers and if it's pressed, it sets it to the key_map output
 * 
 *  
 */


void check_keys(uint8_t scancode, uint8_t data, key_map *output_key) {
    int i;
    
    /* go through letters to see if pressed*/
    for (i = 0; i < NUM_LETTERS; i++) {
        if (key_letters[i].scancode == scancode) {
            *output_key = key_letters[i];
            break;
        }
    }

    /* go through symbols to see if pressed */
    for (i = 0; i < NUM_SYMBOLS && (!output_key->scancode); i++) {
        if (key_symbols[i].scancode == scancode) {
            *output_key = key_symbols[i];
            break;
        }
    }

    /* check if we found a key */
    if (!output_key->scancode) return;
    
    /* update the state of the letter found */
    if (data & (1 << 7) && (scancode != CAPS_LOCK)) {
        output_key->state = 0;
    } else {
        output_key->state = 1;
    }
    
}
/* unsigned char check_states(int found_letter_key, key_map *output)
 * Inputs: int found_letter_key, key_map *output
           
 * Return Value: unsigned char c
 * Function: checks the state of the shift keys and caps_lock key
 *  to see if we need to print the unshifted or shifted character.
 * 
 *  
 */

uint8_t check_states(key_map *output) {

    uint8_t c;
    int found_letter_key;

    /* check to see if we found a letter key */
    if (output->unshifted >= 'a' && output->unshifted <= 'z') {
        found_letter_key = 1;
    } else {
        found_letter_key = 0;
    }

    /* Check our states to see if we need to print the unshifted or shifted char */
    if (key_state[L_SHIFT_IDX].state || key_state[R_SHIFT_IDX].state) {
        if (key_state[CAPS_LOCK_IDX].state) {
            c = (found_letter_key ? output->unshifted : output->shifted);
        } else {
            c = output->shifted;
        }
    } 
    else if (key_state[CAPS_LOCK_IDX].state) {
        c = (found_letter_key ? output->shifted : output->unshifted);
    }
    else {
        c = output->unshifted;
    }


    return c;
}
/* void backspace_pressed()
 * Inputs: none
 * Return Value: void
 * Function: handles the backspace key by deleting the last character(s) in the buffer
 * 
 *  
 */

void backspace_pressed() {
    int i;
    /* get our current buffer idx for our terminal */
    int buf_idx = terminal_ctx[terminal_shown].buf_idx;

    if (!buf_idx) return;

    buf_idx--;
    /* if we see a tab in buf that means we backspace 3 times*/
    if (terminal_ctx[terminal_shown].keyboard_buf[buf_idx] == '\t') {
        for (i = 0; i < 3; i++, buf_idx--) {
            putc('\b');
            terminal_ctx[terminal_shown].keyboard_buf[buf_idx] = 0;
        }
    } else {
        terminal_ctx[terminal_shown].keyboard_buf[buf_idx] = 0;
    }

    /* update the buf_idx for the struct */
    terminal_ctx[terminal_shown].buf_idx = buf_idx;
}

/* void add_to_buffer(unsigned char c, key_map *output)
 * Inputs: unsigned char c, key_map *output
 * Return Value: void
 * Function: adds the character c to the keyboard buffer if appropriate,
 * after checking for tab and backspace and adding/removing spaces from the buffer accordingly.
 *  
 */
void add_to_buffer(unsigned char c, key_map *output) {
    int i;
    /* get our current buffer idx for our terminal */
    int buf_idx = terminal_ctx[terminal_shown].buf_idx;
    
    switch(output->scancode) {
        case TAB:
        {
            /* If we see a tab, add 3 spaces to the buffer and screen */

            /* Check if adding 3 spaces and a tab will overflow the buffer*/
            if ((buf_idx + 4) >= BUF_SIZE - 1) break; 
            
            
            for (i = 0; i < 3; i++, buf_idx++) {
                terminal_ctx[terminal_shown].keyboard_buf[buf_idx] = ' ';
                putc(' ');
            }
            terminal_ctx[terminal_shown].keyboard_buf[buf_idx] = '\t'; // Insert '\t' into the buffer
            buf_idx++;
            putc(' '); // output a space instead of the tab character
            break;
        }
        case BACKSPACE:
        {
            // If it's a backspace, we output the backspace char
            putc('\b');
            break;
        }
        case ENTER:
        {   
            /* Don't allow enter spam */
            if (!terminal_ctx[terminal_shown].enter_flag) {
                /* Get saved_commands ptr from terminal info */
                saved_commands = terminal_ctx[terminal_shown].saved_commands;

                /* Save command if command is not empty */
                if (buf_idx > 0) { 
                    memcpy(saved_commands[top_scommands_idx].command, terminal_ctx[terminal_shown].keyboard_buf, BUF_SIZE);
                    saved_commands[curr_scommands_idx].history = 0;
                    saved_commands[top_scommands_idx].history = 1;
                    if (++top_scommands_idx >= SAVED_COMMANDS_SIZE) top_scommands_idx = 0;
                    curr_scommands_idx = top_scommands_idx;
                }

                /* Enter */
                terminal_ctx[terminal_shown].keyboard_buf[buf_idx] = '\n';
                buf_idx++;
                terminal_ctx[terminal_shown].enter_flag = 1;
                putc('\n');
            }
            break;
        }
        default:
        {
            // For other characters, add them to the buffer and print
            terminal_ctx[terminal_shown].keyboard_buf[buf_idx] = c;
            buf_idx++;
            putc(c);
            break;
        }
    }

    /* update the buf_idx for the struct */
    terminal_ctx[terminal_shown].buf_idx = buf_idx;
}


/* void add_to_buffer_scommand(unsigned char c)
 * Inputs: unsigned char c
 * Return Value: void
 * Function: adds the character c to the keyboard buffer (used for saved_commands)
 */
void add_to_buffer_scommand(unsigned char c) {
    /* get our current buffer idx for our terminal */
    int buf_idx = terminal_ctx[terminal_shown].buf_idx;

    // For other characters, add them to the buffer and print
    terminal_ctx[terminal_shown].keyboard_buf[buf_idx] = c;
    buf_idx++;
    putc(c);

    /* update the buf_idx for the struct */
    terminal_ctx[terminal_shown].buf_idx = buf_idx;
}


/* int check_buffer_over_flow()
 * Inputs: none
 * Return Value: 1 if buffer is full, 0 if not
 * Function: checks if the buffer is full and sets the buffer full flag accordingly
 *  
 */
int check_buffer_overflow(key_map *output_key) {

    if ((terminal_ctx[terminal_shown].buf_idx) < BUF_SIZE - 1) return 0;

    /* wait to receive enter before resetting our buffer idx (this is the only character that can work if we are i = 126)*/
    if(output_key->scancode == ENTER) {
        /*add our \n to the end of our string idx 127 and output it*/
        terminal_ctx[terminal_shown].keyboard_buf[BUF_SIZE - 1] = '\n';
        putc('\n');
        terminal_ctx[terminal_shown].enter_flag = 1;
    }

    return 1;
        
}

/* type_command();
 * Inputs: dir - 1 - user pressed up
 *               0 - user pressed down
 * Return Value: None
 * Function: types the saved command based on current command index */
void type_command(int32_t dir) {
    int i = 0;

    /* Get saved_command ptr from terminal info */
    saved_commands = terminal_ctx[terminal_shown].saved_commands;

    /* Save command */
    memcpy(saved_commands[curr_scommands_idx].command, terminal_ctx[terminal_shown].keyboard_buf, BUF_SIZE);

    /* User pressed up */
    if (dir) {
        clear_keyboard_buf_scommand();

        /* If saved command idx reaches 0, loop back around array */
        if (curr_scommands_idx == 0) curr_scommands_idx = SAVED_COMMANDS_SIZE;

        /* Check saved commands until we find one that is viewable */
        while (!saved_commands[--curr_scommands_idx].history) {
            if (++i > SAVED_COMMANDS_SIZE) return; /* Return in case we enter infinite loop */
            if (curr_scommands_idx == 0) curr_scommands_idx = SAVED_COMMANDS_SIZE;
        }
    }
    /* User pressed down (check if we aren't at the latest saved command) */
    else if (curr_scommands_idx != top_scommands_idx) {
        clear_keyboard_buf_scommand();

        /* If saved command idx reaches max size, loop back around array */
        if (curr_scommands_idx == SAVED_COMMANDS_SIZE - 1) curr_scommands_idx = -1;

        /* Check saved commands until we find one that is viewable */
        while (!saved_commands[++curr_scommands_idx].history) {
            if (curr_scommands_idx == SAVED_COMMANDS_SIZE - 1) curr_scommands_idx = -1;
            if (curr_scommands_idx == top_scommands_idx) return; /* Return in case we hit top scommand */
            if (++i > SAVED_COMMANDS_SIZE) return; /* Return in case we enter infinite loop */
        }
    }
    /* We are at the latest saved command, don't do anything */
    else return;

    /* Add each character from saved command to screen and keyboard buffer */
    for (i = 0; i < BUF_SIZE; i++) {
        if (saved_commands[curr_scommands_idx].command[i] != 0)
            add_to_buffer_scommand(saved_commands[curr_scommands_idx].command[i]);
        else break;
    }
}

/* clear_keyboard_buf_scommand();
 * Inputs: None
 * Return Value: None
 * Function: clears the keyboard buffer and resets its index (used for saved_commands) */
void clear_keyboard_buf_scommand() {
    int i;
    for (i = 0; i < BUF_SIZE; i++) {
        if (terminal_ctx[terminal_shown].keyboard_buf[i]) {
            terminal_ctx[terminal_shown].keyboard_buf[i] = 0;
            putc('\b');
        }
    }

    terminal_ctx[terminal_shown].buf_idx = 0;
    terminal_ctx[terminal_shown].terminal_x = 0;
}

