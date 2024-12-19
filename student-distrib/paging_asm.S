# paging_asm.S - Set up paging
# vim:ts=4 noexpandtab

#define ASM  1
#include "paging.h"

.text

.globl load_page_directory, enable_paging, flush_tlb
.globl page_directory, first_page_table

# Declaring paging functions
/* load_page_directory - Load the page directory into the CR3 register
 * @param page_directory - The page directory to load
*/
load_page_directory:
    pushl %ebp
    movl %esp, %ebp
    movl 8(%esp), %eax
    movl %eax, %cr3
    movl %ebp, %esp
    popl %ebp 
    ret

/* enable_paging - Enable paging by setting the PG and PE bits in CR0
 * and the PSE bit in CR4
*/
enable_paging:
    pushl %ebp
    movl %esp, %ebp
    # Enable PSE
    movl %cr4, %eax
    orl $0x00000010, %eax
    movl %eax, %cr4
    # Enable PG and PE
    movl %cr0, %eax
    movl %cr0, %eax
    orl $0x80000001, %eax
    movl %eax, %cr0
    # Enable PGE
    movl %cr4, %eax
    orl $0x00000080, %eax
    movl %eax, %cr4
    # Teardown
    movl %ebp, %esp
    popl %ebp
    ret
/* flush_tlb - Flush the TLB by reloading the CR3 register
    * This is done by writing the value of CR3 back to CR3
*/
flush_tlb:
    # Don't forget to flush...
    movl %cr3, %eax
    movl %eax, %cr3
    ret


# Declaring page directories and tables

.align PAGE_SIZE 
page_directory:
_page_directory:
    .rept NUM_PD
    .long 0
    .endr

.align PAGE_SIZE
first_page_table:
_first_page_table:
    .rept NUM_PT
    .long 0
    .endr

