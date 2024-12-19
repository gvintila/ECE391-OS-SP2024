/* Header file for our terminal Driver */
#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"
#include "kboard.h"

/* define indices of stdin and stdout for fd*/
#define FD_STDIN_IDX 0
#define FD_STDOUT_IDX 1

#define NUM_TERMINALS 3

/* Terminal Struct to keep track of state per terminal */
typedef struct {
    uint8_t keyboard_buf[BUF_SIZE];
    saved_command_t saved_commands[SAVED_COMMANDS_SIZE];
    int32_t curr_scommands_idx;
    int32_t top_scommands_idx;
    uint8_t buf_idx;
    uint32_t screen_x;
    uint32_t screen_y;
    uint32_t terminal_x;
    volatile uint32_t enter_flag;
} terminal_t;

/* define for terminals */
terminal_t terminal_ctx[NUM_TERMINALS];

/* opens terminal */
extern int32_t terminal_open(const uint8_t* filename);

/* closes terminal */
extern int32_t terminal_close(int32_t fd);

/*should read input from user after pressing enter*/
extern int32_t terminal_read(int32_t fd, void *buf, int32_t nbytes);

/* should take a string of chars with the number of characters to write*/
extern int32_t terminal_write(int32_t fd, const void *buf, int32_t nbytes);

/* bad read */
extern int32_t bad_read();



#endif  /* _TERMINAL_H */
