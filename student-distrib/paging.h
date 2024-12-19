/* paging.h - Functions for initializing paging
 * vim:ts=4 noexpandtab
 */

#ifndef _PAGING_H
#define _PAGING_H

#include "pid.h"
#include "types.h"
#include "lib.h"
#include "scheduler.h"

/* Paging */

/* Size in bytes of a pde/pt/page */
#define PAGE_SIZE       4096                    /* 4KB */
#define BIG_PAGE_SIZE   PAGE_SIZE * NUM_PT      /* 4MB */

/* Number of page directories and page tables */
#define NUM_PD      1024
#define NUM_PT      1024

/* Bit number where base address is initialized */
#define PAGE_ADDR_BITS      20
#define BASE_ADDR_BITS      12

/* Page directories associated with video memory and kernel data */
#define VMEM_PD             0
#define KERNEL_PD           1

/* Page tables associated with screen vmem and backing terminal vmem within first_page_table */
#define SCREEN_VMEM_PT      VIDEO / PAGE_SIZE       /* 184 */
#define TERM_VMEM_PT        186

/* Defines where in memory kernel block starts and ends */
#define KERNEL_START        KERNEL_PD * BIG_PAGE_SIZE
#define KERNEL_END          (((KERNEL_PD) + 1) * BIG_PAGE_SIZE) - 1

/* Defines where in memory process starts and ends */
#define PROCESS_START       2 * BIG_PAGE_SIZE
#define PROCESS_END         (1 + PID_NUM) * BIG_PAGE_SIZE

/* Page directory associated with user programs */
#define USR_PRGM_PD         128 / 4                 /* 32 */

/* Start user vidmap after program image */
#define VIDMAP_PD           34
#define VIDMAP_PT           0

/* Address in memory for vidmap page */
#define VIDMAP_ADDR         ( VIDMAP_PD * BIG_PAGE_SIZE )  

/* Defines where in virtual memory user program starts and ends */
#define USR_PAGE            0x08000000
#define USR_PRGM_START      0x08048000
#define USR_PRGM_OFFSET     0x00048000


/* Malloc */

/* Malloc PDs */
#define MALLOC_PD_START     40
#define MALLOC_PD_END       50
#define MALLOC_PD_SIZE      (MALLOC_PD_END - MALLOC_PD_START)

/* Malloc map size - takes the number of different order buddy block sizes for 
 * each PD and multiplies it by the total number of PDs to get the total size for
 * the binary tree that makes up our mapping for malloc pages */
#define MMAP_SIZE           (1024 + 512 + 256 + 128 + 64 + 32 + 16 + 8 + 4 + 2 + 1)

/* Max order of buddy block (log2(1024)) */
#define MAX_ORDER           10

/* Binary tree macros */
#define TREE_LEFT(i)        (2*i + 1)
#define TREE_RIGHT(i)       (2*i + 2)
#define TREE_PARENT(i)      ((i - 1) / 2)

#ifndef ASM


/* PDEs/PTEs */

/* See IA-32 3.7.6 for full diagram of structs defined here */

typedef struct __attribute__((packed)) pde {
    uint32_t present                : 1;
    uint32_t rw                     : 1;
    uint32_t user                   : 1;
    uint32_t write_through          : 1;
    uint32_t cache                  : 1;
    uint32_t accessed               : 1;
    uint32_t reserved               : 1;
    uint32_t size                   : 1;
    uint32_t global                 : 1;
    uint32_t avail                  : 3;
    uint32_t pt_base_addr           : 20;
} pde_t;

typedef struct __attribute__((packed)) pte {
    uint32_t present                : 1;
    uint32_t rw                     : 1;
    uint32_t user                   : 1;
    uint32_t write_through          : 1;
    uint32_t cache                  : 1;
    uint32_t accessed               : 1;
    uint32_t dirty                  : 1;
    uint32_t ptai                   : 1;
    uint32_t global                 : 1;
    uint32_t avail                  : 3;
    uint32_t page_base_addr         : 20;
} pte_t;

/* Initializing page directories */
pde_t page_directory[NUM_PD] __attribute__((aligned(PAGE_SIZE)));

/* Initializing page tables */
pte_t first_page_table[NUM_PT] __attribute__((aligned(PAGE_SIZE)));

/* Initialize user page table */
pte_t vidmap_page_table[NUM_PT] __attribute__((aligned(PAGE_SIZE)));


/* Malloc */

/* We'll be using the buddy system for the malloc implementation. The smallest chunk that can be allocated
 * is a page (4KB) and the largest chunk that can be allocated is a big page (4MB). We will define our metadata
 * containing information about our buddy blocks as an array but traversing it as if it was a binary tree. */

/* Struct used for defining malloc page metadata in a binary tree */
typedef struct __attribute__((packed)) malloc_map {
    uint32_t unavail : 1;       /* Current block is unavailable to use at all (1 or 0) */
    uint32_t split : 1;         /* Indicates whether block was split up into smaller chunks (1 or 0) */
} malloc_map_t;

/* Malloc page metadata - initialized as an array but we will traverse it as a binary tree */
malloc_map_t malloc_metadata[MALLOC_PD_SIZE][MMAP_SIZE];


/* Functions */

/* Initializing paging variables including PDs and PTs */
extern void paging_init();

/* Load user program at specified process (PID) offset */
extern void page_user_program(uint32_t pid);

/* Load vidmap to specified address from PID */
extern void page_vidmap(uint32_t pid);

/* Copy current terminal vmem to screen vmem or vice versa */
extern void copy_term_vmem(uint32_t terminal, uint32_t screen);

/* Get pointer to terminal vmem */
extern void* get_term_vmem(uint32_t terminal);

/* Change vidmap for vmem upon schedule */
extern void change_vidmap(uint32_t terminal);

/* Recursive binary tree algorithm used for finding malloc block */
extern int32_t malloc_tree(int32_t tree_idx, int32_t curr_order, int32_t target_order, int32_t pd);

/* Load the base address of the PD into the CR3 register for the MMU */
extern void load_page_directory(pde_t *pd);

/* Enable paging by setting the paging bit on the CR0 register */
extern void enable_paging();

/* Flush TLB by moving CR3 register back in */
extern void flush_tlb();

#endif /* _PAGING_H */

#endif /* ASM */
