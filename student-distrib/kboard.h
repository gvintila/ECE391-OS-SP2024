/*
    kboard.h - Define functions and global variable S
    for keyboard interrupt.

*/

#ifndef _KBOARD_H
#define _KBOARD_H

#include "x86_inter.h"
#include "lib.h"
#include "scheduler.h"

/*Define ports*/
#define  KB_DATA_PORT          0x60
#define  KB_COMMAND_PORT       0x64

/* Define bytes to send or receive from keyboard*/

// bytes we can receive FROM keyboard
#define  KB_ACK       0xFA   // ack for when we send command to keyboard
#define  KB_RESEND    0xFE   // receive this if keyboard wants to repeat the command we want to send

// commands to send TO keyboard
#define  KB_SCANCODE 0xF0 // sets the scan code mode we want to use

/* Define constants */
#define  SCAN_MODE   0x01  // scan code mode for our keycoard

/* Define indices into our key_state for easier access*/
#define  L_SHIFT_IDX    0
#define  R_SHIFT_IDX    1
#define  CTRL_IDX       2
#define  CAPS_LOCK_IDX  3
#define  ALT_IDX        4
#define  F1_IDX         5
#define  F2_IDX         6
#define  F3_IDX         7   
#define  UP_IDX         8
#define  DOWN_IDX       9


/* different cases that affect output of characters*/
#define  ENTER      0x1C
#define  BACKSPACE  0x0E
#define  TAB        0x0F
#define  CAPS_LOCK   0x3A

/* keyboard buffer size */
#define  BUF_SIZE   128

/* Saved commands define */
#define  SAVED_COMMANDS_SIZE    32

/* Thank you C :) */
#define  TERMINAL_COUNT         3
#define  MAX_FILE_NAME_LENGTH   32

#ifndef ASM

/*Define mapping of keys */
typedef struct {
    uint32_t scancode;
    uint8_t unshifted; // each key can be mapped to 2 different characters or symbols
    uint8_t shifted;
    uint8_t state;
}key_map;

/* for toggle keys such as caps, alt, shift*/
typedef struct {
    uint32_t scancode;
    uint8_t state; // state 1 means it has been pressed and 0 it has not
}key_state_map;

/* Saved commands */
typedef struct saved_command {
    uint8_t command[BUF_SIZE];          /* Saved command associated with entry */
    char history;                       /* Shows if command should be shown in command history (1 or 0) */
} saved_command_t;

saved_command_t *saved_commands;        /* Array of saved commands (ptr to terminal info) */
int32_t top_scommands_idx;              /* The latest saved command entry that we want to fill up */
int32_t curr_scommands_idx;             /* The current saved command entry that we want to view/edit */


#define     NUM_LETTERS       26
#define     NUM_SYMBOLS       25
#define     NUM_STATE_KEYS    10
#define     TOTAL_KEYS        NUM_LETTERS + NUM_SYMBOLS + NUM_STATE_KEYS


/* Initialize Keyboard Interrupts*/
extern void enable_kboard_interrupt();

extern void kboard_init();

/* pressed keys */
extern key_map key_letters[NUM_LETTERS];

/* pressed symbols */
extern key_map key_symbols[NUM_SYMBOLS];

/* different key states that can be pressed such as SHIFT and TAB*/
extern key_state_map key_state[NUM_STATE_KEYS];

/* ack from keyboard*/
extern volatile int kboard_ack;

/* waits for ack*/
extern void wait_ack();

/* THE REST ARE HELPER FUNCTIONS FOR KEYBOARD HANDLER*/

extern void update_state_keys(uint8_t scancode, uint8_t data);

extern void check_keys(uint8_t scancode, uint8_t data,  key_map *output_key);

extern uint8_t check_states(key_map *output_key);

extern void backspace_pressed();

extern void add_to_buffer(uint8_t c, key_map *output);

extern int check_buffer_overflow();

extern void type_command(int32_t dir);

void clear_keyboard_buf_scommand();

#endif

#endif /* ASM */

