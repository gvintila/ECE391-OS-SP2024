/* filsystems.c - Functions for initializing and storing
 * information regarding pcbs and kernel stacks
 * vim:ts=4 noexpandtab
 */

#include "pid.h"
#include "rtc.h"
#include "filesystem.h"
#include "lib.h"
#include "terminal.h"

/* Initializing file operation tables */
file_ops_t rtc_op_table = {&rtc_open, 
                           &rtc_read, 
                           &rtc_write, 
                           &rtc_close};

file_ops_t direc_op_table = {&directory_open, 
                             &directory_read, 
                             &directory_write, 
                             &directory_close};

file_ops_t file_op_table = {&file_open, 
                            &file_read, 
                            &file_write, 
                            &file_close};

/* init_pids
    * DESCRIPTION: Initializes all PIDs at their respective locations in memory
    *
    * INPUTS: None
    * OUTPUTS: None
    * RETURN VALUE: 0
    * 
*/
int32_t init_pids() {
    int i;

    /* Clears PCBs in memory with 0 */
    for (i = 0; i < PID_NUM; i++) memset(get_pid_loc(i), 0, sizeof(pcb_t));

    /* Set PCBs to their respective locations in memory */
    for (i = 0; i < PID_NUM; i++) pcbs[i] = (pcb_t*)(get_pid_loc(i));
    
    curr_pid = SHELL_PID;

    return 0;
}

/* get_pid_loc
    * DESCRIPTION: Get location of specified PID within memory
    *
    * INPUTS: PID index
    * OUTPUTS: Starting location of PID index within memory as a void pointer
    * RETURN VALUE: void*
    * 
*/
void* get_pid_loc(uint32_t pid) {
    /* Check if pid is out of range, return NULL pointer */
    if (pid < 0 || pid >= PID_NUM) {
        printf("get_pid_loc - PID out of range!\n");
        return (void*)NULL;
    }
    /* Return pointer based on end of kernel page */
    return (void*)((KERNEL_END - ((pid + 1) * (PID_SIZE))) + 1);
}

/* get_pstack_loc
    * DESCRIPTION: Get location of specified process (PID) stack within memory
    *
    * INPUTS: PID index
    * OUTPUTS: Starting location of pstack within memory as a void pointer
    * RETURN VALUE: void*
    * 
*/
void* get_pstack_loc(uint32_t pid) {
    /* Check if pid is out of range, return NULL pointer */
    if (pid < 0 || pid >= PID_NUM) {
        printf("get_pstack_loc - PID out of range!\n");
        return (void*)NULL;
    }
    /* Return pointer based on end of kernel page */
    return (void*)(KERNEL_END - (pid * PID_SIZE));
}

/* get_avail_pid
    * DESCRIPTION: Returns lowest index available PID, if none, return -1
    *
    * INPUTS: None
    * OUTPUTS: Lowest index available PID
    * RETURN VALUE: PID index as int32_t or -1
    * 
*/
int32_t get_avail_pid() {
    int i;
    /* WE SHOULD MAKE SURE THIS FUNCTION ALWAYS RETURNS CORRECT INFORMATION!!! - gorg
     * Potential problem: one PID wants to create a new process then calls this, but
     * then scheduler immedietely switches to another process creating a new process which could
     * make the previous function call inaccurate. So we would have to make sure this is atomic.
     * Or maybe this isn't actually a problem idk. */
    for (i = 0; i < PID_NUM; i++) if (pcbs[i]->in_use == 0) return i;
    /* No PIDs are currently available */
    printf("Max processes opened!\n");
    return -1;
}

/* init_pcb
    * DESCRIPTION: Initializes the file descriptor array for a given PID
    *
    * INPUTS: pid - the PID to initialize the file descriptor array for
    * OUTPUTS: None
    * RETURN VALUE: None
    *
*/
void init_pcb(uint32_t pid) {
    /* Error check */
    if (pid < 0 || pid > PID_NUM) printf("init_pcb - PID out of range!\n");
    else {
        /* Clear the pcb before initializing the fda (useful for when we halt a process) */
        clear_pcb(pid);
        pcbs[pid]->in_use = 1;
        curr_pid = pid;

        /* Adriel -> I think we can just call terminal_open in here without a sys call */
        terminal_open((const uint8_t*)"dummy"); // dummy for now, i dont understand why terminal open needs a filename
    }
}

/* clear_pcb
    * DESCRIPTION: Clears a PCB block during halt.
    *
    * INPUTS: pid - the PID to clear the PCB for
    * OUTPUTS: None
    * RETURN VALUE: None
    *

*/
void clear_pcb(uint32_t pid) {
    /* Error check */
    if (pid < 0 || pid > PID_NUM) printf("clear_pcb - PID out of range!\n");
    else {
        memset(get_pid_loc(pid), 0, sizeof(pcb_t));
        int i;
        for (i = 0; i < FD_ARRAY_SIZE; i++) fda_spaces[pid][i] = 0;
        fda_full[pid] = 0;
    }
}
