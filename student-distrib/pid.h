/* filesystem.h - Defines structs/functions to help
 * for pcb functions and kernel stacks
 * vim:ts=4 noexpandtab
 */

#ifndef _PID_H
#define _PID_H

#include "types.h"
#include "paging.h"
#include "syscall.h"
#include "filesystem.h"
#include "kboard.h"

/* Defines for PIDs */
#define PID_SIZE            8192            /* Size of a PID is 8kb in memory */
#define PID_NUM             6               /* Total number of PIDs/PCBs within memory */

/* PID nums associated with respectives PIDs */
#define SHELL_PID           0                           

/* Memory location associated with respective PIDs */
#define SHELL_PID_LOC       (KERNEL_END - (1 * (PID_SIZE))) + 1

/* Adriel -> Memory location assosciated with each Kernel stack for each PIDs */
#define SHELL_STACK         KERNEL_END

/* Defines for file descriptors */
#define FD_ARRAY_SIZE       8               /* Total size of fd array */

#define MAX_FILE_NAME_LENGTH    32

/* Thank you C :) */
#define BUF_SIZE            128

#ifndef ASM

/* Saved registers struct */
typedef struct saved_regs {
    int32_t eflags;
    int32_t eip; 
    int32_t esp; 
    int32_t ebp; 
    int32_t eax; 
    int32_t ebx; 
    int32_t ecx; 
    int32_t edx; 
    int32_t esi; 
    int32_t edi; 
} saved_regs_t;

/*
offset 0x0
offset 0x4
offset 0x8
offset 0xC
offset 0x10
offset 0x14
offset 0x18
offset 0x1C
offset 0x20
offset 0x24
*/

/* File operations table - fops table */
typedef struct file_ops {
    int32_t (*open)(const uint8_t* fname);
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*close)(int32_t fd);
} file_ops_t;

/* Opened fd struct */
typedef struct fd_file {
    file_ops_t fops_table;      /* File operations jump table */
    uint32_t inode;             /* inode number for file */
    uint32_t file_pos;          /* Keeps track of where user is reading from in file */
    uint32_t flags;             /* Says if fd is in use */
} fd_file_t;

/* PCB struct */
typedef struct __attribute__((packed)) pcb {
    int32_t in_use;                                 /* Showcases if current PCB/PID is already being used by a running process (1 or 0) */
    int32_t shell;                                  /* Showcases if current PCB/PID is running shell (1 or 0) */
    int32_t vidmap;                                 /* Showcases if process is using user vmem (1 or 0) */
    int32_t pid;                                    /* Used for showing PID number that pcb is stored in */
    int32_t parent_pid;                             /* Used for showing parent process, -1 if process is root */
    int32_t terminal;                               /* Used for showing which terminal current PCB/PID belongs to */
    volatile int exception;                         /* Shows if an exception was thrown during process execution and used for squashing (1 or 0) */
    saved_regs_t curr_regs;                         /* PCBs current registers */
    fd_file_t fd_array[FD_ARRAY_SIZE];              /* fd_array (fda) storing file descriptors for current PID */
    int32_t curr_executable_fd;                     /* Stores index (fd) of the current executable that is running, -1 of process is root */
    char args[BUF_SIZE];                            /* Pointer to process args */
} pcb_t;

/* Initializing file operation tables */
file_ops_t rtc_op_table;
file_ops_t direc_op_table;
file_ops_t file_op_table;

/* Shows which PID is currently in use */
volatile uint32_t curr_pid;

/* Temp registers will be saved after ctx switching from scheduler to base shell */
saved_regs_t temp_registers;

/* Initializing PCBs */
pcb_t *pcbs[PID_NUM];

/* Shows available spaces in fd_array */
uint32_t fda_spaces[PID_NUM][FD_ARRAY_SIZE];
uint32_t fda_full[PID_NUM];

/* Initializes all PIDs at their respective locations in memory */
extern int32_t init_pids();

/* Get location of specified PID within memory */
extern void* get_pid_loc(uint32_t pid);

/* Get location of specified process (PID) stack within memory */
extern void* get_pstack_loc(uint32_t pid);

/* Returns lowest index available PID, if none, return -1 */
extern int32_t get_avail_pid();

/* Initializes fda with stdin and stdout for passed PID */
extern void init_pcb(uint32_t pid);

/* Clears a PCB block (UPDATE THIS FOR SCHEDULING!) */
extern void clear_pcb(uint32_t pid);

/* Context switch function to help save and restore registers and push IRET context to stack */
extern void exec_context_switch(saved_regs_t* curr_ctx, saved_regs_t* to_ctx);

/* Context switch function used by halt and scheduler to help save and restore registers and push context to stack */
extern void halt_context_switch(saved_regs_t* curr_ctx, saved_regs_t* to_ctx);

/* Saves registers in current context */
extern void save_ctx_regs(saved_regs_t* curr_ctx);

#endif /* _PID_H */

#endif /* ASM */
