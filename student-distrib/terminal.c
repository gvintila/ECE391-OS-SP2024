/*
    terminal.c - driver code for our user space shell terminal


*/

#include "terminal.h"
#include "kboard.h"
#include "pid.h"
#include "scheduler.h"

static void clear_keyboard_buf();

/* int terminal_open()
 * Inputs: None
 * Return Value: 0 for success
 * Function: opens the terminal
 * */
int32_t terminal_open(const uint8_t* filename) {
    /* check if we can add these 2 files */
    if (fda_spaces[curr_pid][FD_STDIN_IDX]) {
        printf("STDIN for FD is already filled\n");
        return -1;
    } else
     if  (fda_spaces[curr_pid][FD_STDOUT_IDX]) {
        printf("STDOUT for FD is already filled\n");
        return -1;
    }

    /* Stdin corresponds to terminal read (kboard input)*/
    /* Stdout corresponds to terminal write (outputting to terminal )*/

    /* declare file ops for stdin and stdout, making write for stdin 0 and making read for stdout 0 for now, dont know if this is right approach*/
    file_ops_t stdin_fops = {
        .open = &bad_read,
        .read = &terminal_read,
        .write = &bad_read, 
        .close = &bad_read
    };
    file_ops_t stdout_fops = {
        .open = &bad_read,
        .read = &bad_read,
        .write = &terminal_write,
        .close = &bad_read
    };



    /* Initialize stdin and stdout*/
    fd_file_t stdin = {
        .fops_table = stdin_fops,
        .inode = 0,  // ?? WHAT is the inode for terminal? assuming its 0 for now like RTC
        .file_pos = 0,
        .flags = 1, // stdin is in-use
    };

    fd_file_t stdout = {
        .fops_table = stdout_fops,
        .inode = 0,  // ?? WHAT is the inode for terminal? assuming its 0 for now like RTC
        .file_pos = 0,
        .flags = 1, // stdin is in-use
    };

    /* add our structs to proper PCB array */
    pcbs[curr_pid]->fd_array[FD_STDIN_IDX] = stdin;
    pcbs[curr_pid]->fd_array[FD_STDOUT_IDX] = stdout;

    /* update our spaces flag */
    fda_spaces[curr_pid][FD_STDIN_IDX] = 1;
    fda_spaces[curr_pid][FD_STDOUT_IDX] = 1;

    /* Successful terminal open, return 0*/
    return 0;
}

/* int terminal_close()
 * Inputs: None
 * Return Value: 0 for success
 * Function: closes the terminal
 * */
int32_t terminal_close(int32_t fd) {
    /* Error check for out of range fds, should be 0 and 1*/
    if (fd < 0 || fd > 2) {
        return -1;
    } 

    /* empty the proper fd at proper curr_pid */
    pcbs[curr_pid]->fd_array[fd] = (fd_file_t){{0, 0, 0, 0}};
    
    /* update our flags */
    fda_spaces[curr_pid][fd] = 0;

    /* update case where fda was full but not anymore after emptying */
    if (fda_full[curr_pid]) {
        fda_full[curr_pid] = 0;
    }
    
    return 0;
}

/* int terminal_read(int32_t fd, char *buf, uint32_t nbytes);
 * Inputs: uint32_t fd, char *buf, uint32_t nbytes
 * Return Value: number of bytes copied
 * Function: reads from the terminal/keyboard buffer */
int32_t terminal_read(int32_t fd, void *buf, int32_t nbytes) {
    
    uint8_t *buffer = buf;
    int i;
    

    if (fd != 0 || buffer == 0) {
        return -1;
    }
    
    clear_keyboard_buf();

    // wait to see if user has pressed enter or if the buffer is filled
    while(!terminal_ctx[terminal_active].enter_flag) {
       continue;    
    }

    uint32_t flags;
    cli_and_save(flags);
    /* we are here if enter flag = 1 so we copy the keyboard buffer to our buf */
    
    uint8_t* temp;
    int copied = 0;
    int success;

    if (nbytes > BUF_SIZE) {
        nbytes = BUF_SIZE;
    }

    temp = buffer; // this is so we dont lose the initial address of buf

    for (i = 0; i < nbytes; i++, temp++) {
        if (terminal_ctx[terminal_active].keyboard_buf[i] == '\0') {
            break;
        }
        *temp = (uint8_t)(terminal_ctx[terminal_active].keyboard_buf[i]);
        copied++;   
    }
    success = copied;

    /* fill the rest will nulls */
    while(copied < nbytes) {
        *temp = 0;
        copied++;
        temp++;
    }

    clear_keyboard_buf();

    terminal_ctx[terminal_active].enter_flag = 0;
    restore_flags(flags);

    return success;

}
/* int terminal_write(unigned char c);
 * Inputs: unsigned char c
 * Return Value: return num bytes copied, or 0 for error
 * Function: writes to the terminal/video memory */
int32_t terminal_write(int32_t fd, const void *buf, int32_t nbytes) {

    const uint8_t *buffer = buf;

    if (fd != 1) {
        return -1;
    }

    if (!buffer) {
        return -1;
    }
   
    int i;  
    int copied = 0;

    for (i = 0; i < nbytes; i++) {
        // check here for null bytes
        if (buffer[i]) {
            putc(buffer[i]);
        }
        copied++;
    }
    
    terminal_x = 0;
    
    return copied;
}
/* int32_t bad_read();
 * Inputs: None
 * Return Value: None
 * Function: bad read returns -1 */
int32_t bad_read() {
    return -1;
}
/* clear_keyboard_buf();
 * Inputs: None
 * Return Value: None
 * Function: clears the keyboard buffer and resets its index */
static void clear_keyboard_buf() {
    uint32_t flags;
    cli_and_save(flags);
    int i;
    for (i = 0; i < BUF_SIZE; i++) {
        if (terminal_ctx[terminal_active].keyboard_buf[i]) {
            terminal_ctx[terminal_active].keyboard_buf[i] = 0;
            putc('\b');
        }
    }

    terminal_ctx[terminal_active].buf_idx = 0;
    
    restore_flags(flags);
}
