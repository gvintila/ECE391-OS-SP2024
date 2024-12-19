/* paging.c - Functions for initializing paging
 * vim:ts=4 noexpandtab
 */

#include "paging.h"

/* Initializing paging function 
 *
 * Sets each PDE to not present and initializes kernel
 * and video memory PDE to their respective starting values */
void paging_init() {
    int i;

    /* Set each page directory entry (PDE) to not present */
    for (i = 0; i < NUM_PD; i++) {
        // This sets the following flags to the pages:
        //   Supervisor: Only kernel-mode can access them
        //   Write Enabled: It can be both read from and written to
        //   Not Present: The page table is not present
        page_directory[i].rw = 1;

        /* Fill in first page directory with addresses and set flags to: 
         * Supervisor, Write Enabled, and Not Present */
        first_page_table[i].page_base_addr = (i * PAGE_SIZE) >> BASE_ADDR_BITS;
        first_page_table[i].rw = 1;
    }

    /* Set screen vmem page to be Present, Write Enabled, Supervisor */
    first_page_table[SCREEN_VMEM_PT].rw = 1;
    first_page_table[SCREEN_VMEM_PT].present = 1;

    /* Set term vmem pages to be Present, Write Enabled, Supervisor */
    for (i = 0; i < TERMINAL_COUNT; i++) {
        first_page_table[TERM_VMEM_PT + i].rw = 1;
        first_page_table[TERM_VMEM_PT + i].present = 1;
    }

    /* Set first page to present, write enabled, supervisor,
     * and loaded with it's respective page table address  */
    page_directory[VMEM_PD].pt_base_addr = ((unsigned int)first_page_table) >> BASE_ADDR_BITS;
    page_directory[VMEM_PD].rw = 1;
    page_directory[VMEM_PD].present = 1;

    /* Set kernel to present, write enabled, supervisor, 4MB size,
     * and loaded with it's respective page address (4MB-8MB) */
    page_directory[KERNEL_PD].pt_base_addr = KERNEL_START >> BASE_ADDR_BITS;
    page_directory[KERNEL_PD].global = 1;
    page_directory[KERNEL_PD].size = 1;
    page_directory[KERNEL_PD].rw = 1;
    page_directory[KERNEL_PD].present = 1;

    /* Setting up user program page directory */
    page_directory[USR_PRGM_PD].pt_base_addr = (uint32_t)0;
    page_directory[USR_PRGM_PD].user = 1;
    page_directory[USR_PRGM_PD].size = 1;
    page_directory[USR_PRGM_PD].rw = 1;
    page_directory[USR_PRGM_PD].present = 1;

    /* Create new page directory entry where user has access to vidmap (user = 1) */
    page_directory[VIDMAP_PD].pt_base_addr = ((unsigned int)vidmap_page_table) >> BASE_ADDR_BITS;
    page_directory[VIDMAP_PD].rw = 1;
    page_directory[VIDMAP_PD].present = 1;
    page_directory[VIDMAP_PD].user = 1;

    /* Set up vidmap page table for vidmap */
    vidmap_page_table[VIDMAP_PT].page_base_addr = (uint32_t)0;
    vidmap_page_table[VIDMAP_PT].rw = 1;
    vidmap_page_table[VIDMAP_PT].user = 1;

    /* Set up malloc pages */
    for (i = MALLOC_PD_START; i < MALLOC_PD_END; i++) {
        page_directory[i].pt_base_addr = i * BIG_PAGE_SIZE;
        page_directory[i].rw = 1;
        page_directory[i].size = 1;
        page_directory[i].user = 1;
        page_directory[i].present = 0;  /* We will make the pages present once we allocate them */
    }

    //printf("Paging initialized.\n");
}

/* Load user program at specified process (PID) offset 
 *
 * User programs start at 8MB + (process number * 4MB) */
void page_user_program(uint32_t pid) {
    page_directory[USR_PRGM_PD].pt_base_addr = ((2 + pid) * BIG_PAGE_SIZE) >> BASE_ADDR_BITS;
    /* Don't forget to flush... */
    flush_tlb();
}

/* Copy current terminal vmem to main screen vmem or vice versa 
 * Inputs:  terminal - terminal number indicating which vmem page to use
 *          screen - 1 indicates copy from terminal vmem to screen,
 *                   0 indicates copy from screen to terminal vmem
 * Outputs: None */
void copy_term_vmem(uint32_t terminal, uint32_t screen) {
    /* Check if we want to copy from term to main screen vmem */
    if (screen) memcpy((void*)VIDEO, (void*)((TERM_VMEM_PT + terminal) * PAGE_SIZE), PAGE_SIZE);
    /* Else we want to copy from screen to term vmem */
    else memcpy((void*)((TERM_VMEM_PT + terminal) * PAGE_SIZE), (void*)VIDEO, PAGE_SIZE);
}

/* Get pointer to terminal vmem */
void* get_term_vmem(uint32_t terminal) {
    return (void*)((TERM_VMEM_PT + terminal) * PAGE_SIZE);
}

/* Change vidmap for vmem upon schedule
 * Inputs:  terminal - terminal number indicating which vmem page to use
 *
 * Outputs: None */
void change_vidmap(uint32_t terminal) {
    /* Check if we want to display current vidmap on screen */
    if (terminal_active == terminal_shown) vidmap_page_table[VIDMAP_PT].page_base_addr = ((uint32_t)VIDEO) >> BASE_ADDR_BITS;
    /* Else we want to store vidmap in backing terminal vmem */
    else vidmap_page_table[VIDMAP_PT].page_base_addr = ((uint32_t)get_term_vmem(terminal)) >> BASE_ADDR_BITS;
}

/* Recursive binary tree algorithm used for finding malloc block 
 * Inputs: tree_idx - current tree_idx inside binary tree array (malloc_metadata)
 *         curr_order - current order in node of instance
 *         target_order - target order block we want to find
 *         pd - pd we currently want to allocate
 */
int32_t malloc_tree(int32_t tree_idx, int32_t curr_order, int32_t target_order, int32_t pd) {
    /* No block can be allocated from this node */
    if ( malloc_metadata[pd][tree_idx].unavail || (malloc_metadata[pd][tree_idx].split && curr_order == target_order) ) 
        return -1;
    /* This block can be allocated */
    else if (curr_order == target_order) {
        /* Set the current block to be unavailable */
        malloc_metadata[pd][tree_idx].unavail = 1;

        /* Return the pt associated with this block */
        while (curr_order-- > 0) tree_idx = TREE_LEFT(tree_idx);
        return ( tree_idx - (MMAP_SIZE - NUM_PT) );
    }
    else {
        /* Check left and right subtrees and return pt if not -1 */
        int32_t left = malloc_tree(TREE_LEFT(tree_idx), curr_order - 1, target_order, pd);
        
        /* Set the current block to be split if a valid pt was returned */
        if (left != -1) {
            malloc_metadata[pd][tree_idx].split = 1;
            return left;
        }

        /* Do the same thing for right subtree if left was -1 */
        int32_t right = malloc_tree(TREE_RIGHT(tree_idx), curr_order - 1, target_order, pd);
        
        if (right != -1) {
            malloc_metadata[pd][tree_idx].split = 1;
            return right;
        }

        /* If left and right subtrees are -1, return -1 */
        return -1;
    }
}
