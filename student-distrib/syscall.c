/*
    Filename: syscall.c
              Handler for each type of system call

*/

#include "syscall.h"

/* syscall_halt
 * 
 * Terminate the current process and return to parent process (shell)
 * Inputs: status - value to return to parent process
 * Outputs: 0 if successful
 
*/
int32_t syscall_halt (uint8_t status) {
    /* cli for critical section */
    cli();

    /* Check if we are trying to halt a base shell */
    if (curr_pid == base_processes[terminal_active]) {
        /* Make sure that base PID gets picked up again when going through execute */
        pcbs[curr_pid]->in_use = 0;

        /* Update scheduler */
        active_processes[terminal_active] = -1;
        base_processes[terminal_active] = -1;

        /* Relaunch shell */
        execute((const uint8_t*)"shell");
    }
    /* Halting a regular (non-base) process */
    else {
        /* Set up variables for context switch */
        uint32_t from_pid = curr_pid;
        uint32_t to_pid = pcbs[from_pid]->parent_pid;
        curr_pid = to_pid;
        pcbs[from_pid]->in_use = 0;
        tss.esp0 = (uint32_t)get_pstack_loc(to_pid);

        /* Update scheduler */
        active_processes[terminal_active] = to_pid;

        /* Close current fd associated with the executable that was called from shell */
        close(pcbs[to_pid]->curr_executable_fd);

        /* Check if vidmap was running in halted program */
        if (pcbs[from_pid]->vidmap) {
            /* Close main screen vidmap if not used on any active process */
            int i;
            int vidmap_flag = 1;
            for (i = 0; i < TERMINAL_COUNT; i++)
                if (active_processes[i] != -1 && pcbs[active_processes[i]]->vidmap)
                    vidmap_flag = 0;
            if (vidmap_flag) {
                vidmap_page_table[VIDMAP_PT].present = 0;
                printf("Vidmap cleared!\n");
            }
        }

        /* Restore parent paging */
        page_user_program(to_pid);

        /* Update parent's ebx with status of child */
        if (pcbs[from_pid]->exception) pcbs[to_pid]->curr_regs.ebx = (uint32_t)256;
        else pcbs[to_pid]->curr_regs.ebx = (uint32_t)status;

        //printf("status: %d\n", pcbs[to_pid]->curr_regs.ebx);

        /* Jump back to parent's (shell's) execute call */
        halt_context_switch(&(pcbs[from_pid]->curr_regs), &(pcbs[to_pid]->curr_regs));
    }
    
    return 0;   /* This return shouldn't trigger */
}

/* syscall_execute
 * 
 * Execute a new program, load it into memory, and switch to it until it terminates.
 * Inputs: command - name of program to execute
 * Outputs: 0 if successful, -1 if not
 */
int32_t syscall_execute (const uint8_t* command) {
    // printf("SYSCALL EXECUTE CALLED, Parameters -> command: %d\n", command);

    /* Check for null argument */
    if (command == 0) return -1;

    /* cli for critical section and save flags */
    uint32_t flags;
    cli_and_save(flags);
    pcbs[curr_pid]->curr_regs.eflags = flags;

    /* Declarations */
    int i;
    int fd;
    uint32_t child_pid;
    char command_name[MAX_FILE_NAME_LENGTH + 1];
    char command_args[BUF_SIZE];

    /* Clean command argument */

    /* Skip empty spaces before first word */
    i = 0;
    while (((char)*command == ' ') || ((char)*command == '\t')) command++;
    while ((char)*command != '\0' && (char)*command != ' ' && (char)*command != '\n') {
        command_name[i] = (char)*command;
        command++;
        i++;
        /* Check if command_name is at end of length then break */
        if (i == MAX_FILE_NAME_LENGTH) break;
    }
    command_name[i] = '\0';

    /* Copy rest of command into args */
    command++;
    strcpy(command_args, (const int8_t*)command);

    // printf("command_name: %s\n", command_name);
    // printf("command_args: %s\n", command_args);

    /* Check command if it's a valid dentry or file type */
    if (check_file_type((const uint8_t*)command_name, 2) == 0) {
        restore_flags(flags);
        return -1;
    }

    /* Check if command can be opened */
    if ((fd = open((const uint8_t*)command_name)) == -1) {
        close(fd);
        printf("Command could not be opened. fd is %d\n", fd);
        restore_flags(flags);
        return -1;
    }

    /* Check program image for validity and find entry point */
    if (check_program_image(fd, (void*)USR_PRGM_START) == -1) {
        close(fd);
        restore_flags(flags);
        return -1;
    } 
    
    void* entry_point = find_program_entry(fd);

    // printf("entry_point: %d\n", (uint32_t)entry_point);

    /* Check if entry point is valid by comparing the top 20 most bits of the address */
    if (((uint32_t)entry_point & 0xFFFFF000) != USR_PRGM_START) {
        close(fd);
        restore_flags(flags);
        return -1;
    }

    /* Check if there are any available PIDs we can use to run new process */
    if ((child_pid = get_avail_pid()) == -1) {
        close(fd);
        restore_flags(flags);
        return 1;
    }

    /* Open correct page for program in memory */
    page_user_program(child_pid);

    /* Load program image into memory */
    int32_t bytes_read;
    if ((bytes_read = load_program_image(fd, (void*)USR_PRGM_START)) == -1) {
        /* If this happens, the parent needs to be paged back */
        close(fd);
        page_user_program(curr_pid);
        restore_flags(flags);
        return 1;
    }

    // printf("Bytes copied for program load: %d\n", bytes_read);
    // printf("Program loaded!\n");

    /* Load new process' PID, PCB, and fd_array and switch to it */
    if ((strncmp((const int8_t*)command_name, (const int8_t*)("shell"), MAX_FILE_NAME_LENGTH) == 0) && (base_processes[terminal_active] == -1)) { /* Executing a base shell */
        /* Close shell in fda */
        close(fd);

        /* Initialize the pcb corresponding to the shell */
        init_pcb(child_pid);
        pcbs[child_pid]->pid = child_pid;
        pcbs[child_pid]->parent_pid = -1;
        pcbs[child_pid]->curr_executable_fd = -1;
        pcbs[child_pid]->shell = 1;
        strcpy(pcbs[child_pid]->args, "");
        pcbs[child_pid]->terminal = terminal_active;

        /* Update active processes in scheduler */
        base_processes[terminal_active] = child_pid;
        active_processes[terminal_active] = child_pid;
        
        /* Debugging - print data for new pcb */
        // printf("New pcb pid: %d\n", pcbs[child_pid]->pid);
        // printf("New pcb parent_id: %d\n", pcbs[child_pid]->parent_pid);
        // printf("Current pid: %d\n", curr_pid);

        /* Set up for IRET call by saving eip to push to stack after */
        pcbs[child_pid]->curr_regs.eip = (uint32_t)entry_point;

        /* Set esp0 equal to the kernel stack of the program we want to execute */
        tss.esp0 = (uint32_t)get_pstack_loc(child_pid);

        /* COMMENT TO READ PRINTS */
        //clear();

        /* Context switch to shell from temp variables (we won't be returning back here) */
        saved_regs_t temp_registers = (saved_regs_t){0};
        exec_context_switch(&temp_registers, &(pcbs[child_pid]->curr_regs));
    } 
    else { /* Executing a child process from shell */
        /* Save parent PID */
        uint32_t parent_pid = curr_pid;

        /* Initialize the pcb corresponding to the child program */
        init_pcb(child_pid);
        pcbs[child_pid]->pid = child_pid;
        pcbs[child_pid]->parent_pid = parent_pid;
        pcbs[parent_pid]->curr_executable_fd = fd;
        strcpy(pcbs[child_pid]->args, command_args);
        pcbs[child_pid]->terminal = terminal_active;

        /* Flag if new process is running a shell */
        if ((strncmp((const int8_t*)command_name, (const int8_t*)("shell"), MAX_FILE_NAME_LENGTH) == 0))
            pcbs[child_pid]->shell = 1;

        /* Update active processes in scheduler */
        active_processes[terminal_active] = child_pid;

        /* Debugging - print data for new pcb */
        // printf("New pcb pid: %d\n", pcbs[child_pid]->pid);
        // printf("New pcb parent_id: %d\n", pcbs[child_pid]->parent_pid);
        // printf("Current pid: %d\n", curr_pid);

        /* Set up for IRET call */
        pcbs[child_pid]->curr_regs.eip = (uint32_t)entry_point;

        /* Set esp0 to the kernel stack of the program we want to execute */
        tss.esp0 = (uint32_t)get_pstack_loc(child_pid);

        /* COMMENT TO READ PRINTS */
        //clear();

        /* Context switch to child process from shell */
        exec_context_switch(&(pcbs[parent_pid]->curr_regs), &(pcbs[child_pid]->curr_regs));
    }

    return 0;
}

/* syscall_read
 * 
 * Reads from fd, calls read operation associated with passed fd in fops_table
 * Inputs: fd - file descriptor
 *         buf - buffer to fill with contents of file
 *         nbytes - number of bytes to store into buffer
 * Outputs: number of bytes copied over to the buffer
 */
int32_t syscall_read (int32_t fd, void* buf, int32_t nbytes) {
    //printf("SYSCALL READ CALLED, Parameters -> fd: %d, buf_ptr: %x, nbytes: %d\n", fd, buf, nbytes);

    /* If fd is out of valid range or fd is currently empty or buf pointer is 0, return -1 */
    if ( fd < 0 || fd >= FD_ARRAY_SIZE || fda_spaces[curr_pid][fd] == 0 || buf == 0)
        return -1;

    /* Call function from respective jump table */
    return (int32_t)pcbs[curr_pid]->fd_array[fd].fops_table.read(fd, buf, nbytes);
}

/* syscall_write
 * 
 * Reads from fd, calls read operation associated with passed fd in fops_table
 * Inputs: fd - file descriptor
 *         buf - buffer to fill with contents of file
 *         nbytes - number of bytes to store into buffer
 * Outputs: 0 for pass, -1 for fail
 */
int32_t syscall_write (int32_t fd, const void* buf, int32_t nbytes) {
    //printf("SYSCALL WRITE CALLED, Parameters -> fd: %d, buf_ptr: %x, nbytes: %d\n", fd, buf, nbytes);

    /* If fd is out of valid range or fd is currently empty or buf pointer is 0, return -1 */
    if ( fd < 0 || fd >= FD_ARRAY_SIZE || fda_spaces[curr_pid][fd] == 0 || buf == 0)
        return -1;

    /* Call function from respective jump table */
    return (int32_t)pcbs[curr_pid]->fd_array[fd].fops_table.write(fd, buf, nbytes);
}

/* syscall_open
 * 
 * Initialize a new file descriptor with data in fd_array in the current PID
 * Inputs: filename - name of file to open
 * Outputs: fd assigned, -1 if failed
 */
int32_t syscall_open (const uint8_t* filename) {
    //printf("SYSCALL OPEN CALLED, Parameters -> filename: %s\n", filename);

    int i;
    dentry_t file_temp;

    /* If filename pointer is 0 or no dentry is found or fda is full, return -1 */
    if ( filename == 0 || (read_dentry_by_name(filename, &file_temp) == -1) || fda_full[curr_pid] )
        return -1;

    fd_file_t new_fd;

    /* Check file type for type respective operations */
    switch (file_temp.file_type) {
        case 0: new_fd.fops_table = rtc_op_table; 
                new_fd.inode = 0; 
                /* Call RTC open right away */
                new_fd.fops_table.open((const uint8_t*)"rtc");
                break;
        case 1: new_fd.fops_table = direc_op_table;    
                new_fd.inode = 0;    
                break;    
        case 2: new_fd.fops_table = file_op_table;
                new_fd.inode = file_temp.index_node_num;
                break;
    }
    new_fd.file_pos = 0;
    new_fd.flags = 1;     /* Defined as "in use" */

    /* Fill in fda with new opened fd */
    for (i = 2; i < FD_ARRAY_SIZE; i++) {
        /* Find first available space */
        if (fda_spaces[curr_pid][i] == 0) {
            fda_spaces[curr_pid][i] = 1;
            pcbs[curr_pid]->fd_array[i] = new_fd;
            /* Check if fda is full */
            if (i == FD_ARRAY_SIZE - 1) fda_full[curr_pid] = 1;
            return i;
        }
    }

    /* fda was actually full and previous for loop failed if this return is reached */
    return -1;
}

/* syscall_close
 * 
 * Empties passed file descriptor in fd_array in current PID
 * Inputs: fd - file descriptor to close
 * Outputs: 0 if sucessful, -1 if not
 */
int32_t syscall_close (int32_t fd) {
    //printf("SYSCALL CLOSE CALLED, Parameters -> fd: %d", fd);

    /* If fd is out of valid range (stdin and stdout should be unclosable!) 
     * or fd is currently empty, return -1 */
    if ( fd < 2 || fd >= FD_ARRAY_SIZE || fda_spaces[curr_pid][fd] == 0 )
        return -1;

    // printf("%d closed!\n", fd);

    /* Empty fd in fd_array */
    pcbs[curr_pid]->fd_array[fd] = (fd_file_t){{0}};
    fda_spaces[curr_pid][fd] = 0;
    /* If fda was previously full, mark as not full anymore after empty */
    if (fda_full[curr_pid]) fda_full[curr_pid] = 0;
    
    
    return 0;
}
/* syscall_getargs
 * 
 * read arguments from the command line into a user-level buffer.
 * Inputs: buf - buffer to store arguments
 *         nbytes - number of bytes to store into buffer
 * Outputs: 0 if successful, -1 if not
 */
int32_t syscall_getargs (uint8_t* buf, int32_t nbytes) {
    //printf("SYSCALL GETARGS CALLED, Parameters -> buf: %x, nbytes: %d", buf, nbytes);

    /* If buf is NULL, return -1 */
    int i;
    char arg[BUF_SIZE];
    strcpy(arg, pcbs[curr_pid]->args);

    //printf("Before filling buffer, arg: %s\n", arg);

    if (buf == 0 || strlen(arg) == 0 || strlen(arg) > BUF_SIZE) {
        return -1;
    }

    /* Clear buffer before copying bytes */
    for (i = 0; i < nbytes; i++) {
        buf[i] = 0;
    }

    /* Check if trying to write more chars than in arg */
    if (nbytes > strlen(arg)) {
        nbytes = strlen(arg);
    }

    for (i = 0; i < nbytes; i++) {
        if (arg[i] == '\0' || arg[i] == ' ' || arg[i] == '\n') break;
        *buf = (uint8_t)(arg[i]);
        buf++;
    }

    return 0;
}
/* syscall_vidmap
 * 
 * Maps the text-mode video memory into user space at a given virtual address.
 * Inputs: screen_start - pointer to the start of the screen
 * Outputs: 0 if successful, -1 if not
 */
int32_t syscall_vidmap (uint8_t** screen_start) {
    /* Check if pointer is valid and screen_start is within program image range */
    if (!screen_start || !((int32_t)screen_start >= USR_PAGE && (int32_t)screen_start <= USR_PAGE + BIG_PAGE_SIZE)) {
        return -1;
    }

    /* Set up vidmap for either main screen or backing term vmem */
    change_vidmap(terminal_active);

    /* Create new page table entry where user also has access to vmem */
    vidmap_page_table[VIDMAP_PT].present = 1;
    *screen_start = (uint8_t*)VIDMAP_ADDR;

    /* Save information in PCB */
    pcbs[curr_pid]->vidmap = 1;

    /* Don't forget to flush... */
    flush_tlb();
    
    return 0;
}

int32_t syscall_set_handler (int32_t signum, void* handler_address) {
    printf("SYSCALL SET HANDLER, Parameters -> signum: %d, handler_addr: %x", signum, handler_address);
    return 0;
}

int32_t syscall_sigreturn (void) {
    printf("SYSCALL SET HANDLER, Paramters -> void ;)");
    return 0;
}

/* Allocate memory with passed size into malloc blocks */
void* syscall_malloc (int32_t size) {
    /* If size is invalid, return NULL */
    if (size <= 0 || size > BIG_PAGE_SIZE) return (void*)NULL;

    /* Declarations */
    int i;
    int32_t curr_pd = 0;
    int32_t curr_pt = 0;
    int32_t target_order = MAX_ORDER;

    /* Determine what block order we want based on size passed */
    while ( (PAGE_SIZE * pow(2, target_order)) > size && target_order > 0) target_order--;

    printf("Allocating size: %d\n", size);

    /* Traverse malloc metadata until first available chunk is found */
    for (i = 0; i < MALLOC_PD_SIZE; i++) {
        /* Start traversing tree */
        curr_pd = MALLOC_PD_START + i;
        curr_pt = malloc_tree(0, MAX_ORDER, target_order, i);
        /* Return address associated with PD and PT */
        if (curr_pt != -1) {
            page_directory[curr_pd].present = 1;
            printf("curr_pd: %d, curr_pt: %d\n", curr_pd, curr_pt);
            printf("Address: %x\n", ( (curr_pd * BIG_PAGE_SIZE) + (curr_pt * PAGE_SIZE) ));
            return (void*)( (curr_pd * BIG_PAGE_SIZE) + (curr_pt * PAGE_SIZE) );
        }
    }

    /* If we're unable to allocate the requested memory size, return NULL */
    return (void*)NULL;
}

/* Free memory with passed pointer and de-allocate buddy blocks */
void syscall_free(void* ptr) {
    /* Calculate associated PD and PT */
    return;
}
