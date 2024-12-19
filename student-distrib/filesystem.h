/* filesystem.h - Defines structs/functions to store data
   about file system in memory
 * vim:ts=4 noexpandtab
 */

#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include "types.h"
#include "pid.h"
#include "syscall.h"

/* Useful offsets to use to index into boot block */
#define NUM_DIR         0
#define NUM_INODES      1
#define DATA_BLOCKS     2
#define FIRST_DIR       16

/* Misc. boot block information */
#define BOOT_RESERVED_BYTES     52      // number of reserved bytes in a boot block
#define MAX_NUM_DIR_ENTRIES     63      // max number of dentries a file system can hold
#define BLOCK_SIZE              4096    // size of a block in bytes
#define BLOCK_SIZE_FOUR_BYTES   1024    // size of a block in bytes divided by 4

/* Misc. index node information */
#define NUM_DATA_BLOCKS     1023        // max number of data blocks (32 bit values) a inode can hold

/* Misc. directory entry information and offsets */
#define DENTRY_RESERVED_BYTES   24      // number of reserved bytes in a directory entry
#define MAX_FILE_NAME_LENGTH    32      // max number of characters for a file name
#define FILE_TYPE               8       // file type in a dentry
#define INODE_NUM               9       // inode #

#ifndef ASM

/* Directory entry struct */
typedef struct directory_entry {
    uint8_t file_name[MAX_FILE_NAME_LENGTH];
    uint32_t file_type;
    uint32_t index_node_num;
} dentry_t;

/* Boot block struct */
typedef struct boot_block {
    uint32_t* boot_addr;
    uint32_t num_dir_entries;
    uint32_t num_inodes;
    uint32_t num_data_blocks;
    dentry_t dir_entries[MAX_NUM_DIR_ENTRIES];
} boot_block_t;

/* Index node struct */
typedef struct __attribute__((packed)) inode_t {
    uint32_t length;
    uint32_t data_blocks[NUM_DATA_BLOCKS];
} inode_t;

/* Boot block information */
boot_block_t boot_data;

/* File commands */
/* Initializes the new opened file descriptor with the proper data */
extern int32_t file_open(const uint8_t *fname);

/* Empties the passed file descriptor in fda */
extern int32_t file_close(int32_t fd);

/* Reads the contents of the file identified by passed fd and copies nbytes to buffer */
extern int32_t file_read(int32_t fd, void* buf, int32_t nbytes);

/* Reads the contents of the file identified by passed fd and copies nbytes to buffer without file_pos */
extern int32_t file_read_nopos(int32_t fd, void* buf, int32_t nbytes);

/* Writes to file, but file system is read-only so we always return -1 */
extern int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);

/* Directory commands */
/* Initialize new opened directory in fda with data */
extern int32_t directory_open(const uint8_t *fname);

/* Reverts the currently open directory to be empty */
extern int32_t directory_close(int32_t fd);

/* Copies the file name of the next file in the currently open directory */
extern int32_t directory_read(int32_t fd, void* buf, int32_t nbytes);

/* Writes to directory, but file system is read-only so we always return -1 */
extern int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes);

/* Initializes the boot block, inodes, and dentries */
extern void read_boot_block(boot_block_t* boot, uint32_t* boot_address);

/* Helper to print out the file name of a specific dentry */
extern void print_file_name(const uint8_t* fname);

/* Helper to print a dentry's file name, file type, and inode number */
extern void print_dentry(dentry_t* dentry);

/* Fills a dentry object with the data corresponding to a particular filename */
extern int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);

/* Fills a dentry object with the data corresponding to a particular file index in boot block */
extern int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);

/* Copies specific number of bytes from a file into a buffer passed in */
extern int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* Gets the length variable marked at the beginning of an inode */
extern int32_t get_inode_length(int32_t fd);

/* Checks if fname defines a file of the expected type */
extern int32_t check_file_type(const uint8_t* fname, uint32_t type);

/* Checks if program image has correct executable format and is readable */
extern int32_t check_program_image(int32_t fd, void* usr_prgm);

/* Loads program image from filesystem into passed user program location in memory */
extern int32_t load_program_image(int32_t fd, void* usr_prgm);

/* Finds entry point for program within the header of the executable */
extern void* find_program_entry(int32_t fd);

#endif /* _FILESYSTEM_H */

#endif /* ASM */

