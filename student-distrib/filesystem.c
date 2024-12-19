/* filsystems.c - Functions for initializing and handling file system
 * vim:ts=4 noexpandtab
 */

#include "filesystem.h"
#include "pid.h"
#include "lib.h"

/* File commands */
/* file_open
 * 
 * Initialize a new file descriptor with data in fd_array
 * Inputs: fname - name of file to open
 * Outputs: fd assigned, -1 if failed
 */
int32_t file_open(const uint8_t *fname) {
    dentry_t file_temp;

    /* If fname pointer is 0 or no dentry is found, return -1 */
    if ( fname == 0 || (read_dentry_by_name(fname, &file_temp) == -1) )
        return -1;

    return 0;
}

/* file_close
 * 
 * Empties passed file descriptor in fd_array
 * Inputs: fd - file descriptor to close
 * Outputs: 0 if sucessful, -1 if not
 */
int32_t file_close(int32_t fd) {
    /* If fd is out of valid range, return -1 */
    if ( fd < 0 || fd >= FD_ARRAY_SIZE )
        return -1;

    return 0;
}

/* file_read
 * 
 * Reads the contents of the file identified by passed fd and copies nbytes to buffer
 * Inputs: fd - file descriptor
 *         buf - buffer to fill with contents of file
 *         nbytes - number of bytes to store into buffer
 * Outputs: number of bytes copied over to the buffer
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes) {
    /* If fd is out of valid range or buf pointer is 0, return -1 */
    if ( fd < 0 || fd >= FD_ARRAY_SIZE || buf == 0 )
        return -1;

    uint32_t bytes_copy = 0;
    uint32_t bytes_read = 0;

    /* Retrieve inode block struct from address indexed by boot address and inode number offset */
    inode_t *inode_block = (inode_t*)(boot_data.boot_addr + ((pcbs[curr_pid]->fd_array[fd].inode + 1) * BLOCK_SIZE_FOUR_BYTES));

    /* Check if end of file is already reached */
    if (pcbs[curr_pid]->fd_array[fd].file_pos >= inode_block->length) return 0;

    /* Calculate maximum number of bytes that can be read from current file position */
    bytes_copy = inode_block->length - pcbs[curr_pid]->fd_array[fd].file_pos;
    if (bytes_copy > nbytes) bytes_copy = nbytes;

    bytes_read = read_data(pcbs[curr_pid]->fd_array[fd].inode, pcbs[curr_pid]->fd_array[fd].file_pos, (uint8_t*)buf, bytes_copy);
    
    /* Error check */
    if (bytes_read == -1) return -1;
    
    /* Increment file position by number of bytes read */
    pcbs[curr_pid]->fd_array[fd].file_pos += bytes_read;

    return bytes_read;
}

/* file_read_nopos
 * 
 * Reads the contents of the file identified by passed fd and copies nbytes to buffer
 * This is different from the normal file_read as we don't save the file_pos and always
 * start at offset = 0
 * 
 * Inputs: fd - file descriptor
 *         buf - buffer to fill with contents of file
 *         nbytes - number of bytes to store into buffer
 * Outputs: number of bytes copied over to the buffer
 */
int32_t file_read_nopos(int32_t fd, void* buf, int32_t nbytes) {
    /* If fd is out of valid range or buf pointer is 0, return -1 */
    if ( fd < 0 || fd >= FD_ARRAY_SIZE || buf == 0 )
        return -1;

    uint32_t bytes_copy = 0;

    /* Retrieve inode block struct from address indexed by boot address and inode number offset */
    inode_t *inode_block = (inode_t*)(boot_data.boot_addr + ((pcbs[curr_pid]->fd_array[fd].inode + 1) * BLOCK_SIZE_FOUR_BYTES));

    bytes_copy = inode_block->length;
    if (bytes_copy > nbytes) bytes_copy = nbytes;

    return read_data(pcbs[curr_pid]->fd_array[fd].inode, 0, (uint8_t*)buf, bytes_copy);
}

/* file_write
 * 
 * Writes to file, but file system is read-only so we always return -1
 * Inputs: fd - file descriptor
 *         buf - buffer to fill with contents of file
 *         nbytes - number of bytes to store into buffer
 * Outputs: -1
 */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

/* Directory commands */
/* directory_open
 * 
 * Initialize new opened directory in fda with data
 * Inputs: fname - name of directory to open
 * Outputs: 0 if sucessful, -1 if not
 * Side Effects: Initializes temporary directory object
 */
int32_t directory_open(const uint8_t *fname) {
    dentry_t directory_temp;

    /* If fname pointer is 0 or no dentry is found, return -1 */
    if ( fname == 0 || (read_dentry_by_name(fname, &directory_temp) == -1) )
        return -1;

    return 0;
}

/* directory_close
 * 
 * Empties currently open directory object
 * Inputs: fname - name of file to open
 * Outputs: 0 if sucessful, -1 if not
 * Side Effects: Empties temporary directory object
 */
int32_t directory_close(int32_t fd) {
    /* If fd is out of valid range, return -1 */
    if ( fd < 0 || fd >= FD_ARRAY_SIZE )
        return -1;

    return 0;
}

/* directory_read
 * 
 * Copy the next file name to print out
 * Inputs: fd - file descriptor
 *         buf - buffer to fill with file name
 *         nbytes - number of bytes to store into buffer
 * Outputs: number of bytes copied over to the buffer
 * Side Effects: Copies next file name in the directory
 */
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes) {
    dentry_t curr_file;

    /* Return 0 repeatedly if additional directory reads are called */
    if (pcbs[curr_pid]->fd_array[fd].file_pos >= boot_data.num_dir_entries) {
        return 0;
    }

    if (read_dentry_by_index(pcbs[curr_pid]->fd_array[fd].file_pos, &curr_file) == -1) {
        return -1;
    }

    strncpy((int8_t*)buf, (int8_t*)(curr_file.file_name), nbytes);
    pcbs[curr_pid]->fd_array[fd].file_pos++;//was ++ after
    
    /* If the length is greater than the max, return the max to avoid printing */
    if(strlen((int8_t*)(curr_file.file_name)) > MAX_FILE_NAME_LENGTH) return MAX_FILE_NAME_LENGTH;

    /* If the length is less than the max, return the value from strlen */
    return strlen((int8_t*)(curr_file.file_name));
}

/* directory_write
 * 
 * Does nothing because read only file system
 * Inputs: fd - file descriptor
 *         buf - buffer to fill with contents of file
 *         nbytes - number of bytes to store into buffer
 * Outputs: number of bytes copied over to the buffer
 * Side Effects: Does nothing (read only)
 */
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

/* read_boot_block
 * 
 * Initializes necessary data corresponding to the boot block and file system
 * Inputs: boot - pointer to boot block to fill with data
 *         boot_address - start address of boot block in memory
 * Outputs: None
 * Side Effects: Initializes boot block object with necessary data
 */
void read_boot_block(boot_block_t* boot, uint32_t* boot_address){
    int dir;

    boot_data.boot_addr = boot_address;
    boot_data.num_dir_entries = *(boot_address + NUM_DIR);
    boot_data.num_inodes = *(boot_address + NUM_INODES);
    boot_data.num_data_blocks = *(boot_address + DATA_BLOCKS);

    for (dir = 0; dir < boot_data.num_dir_entries; ++dir) {
        uint32_t* dentry_block = boot_address + FIRST_DIR*(dir + 1);
        dentry_t new_dentry;
        
        strcpy((int8_t*)new_dentry.file_name, (int8_t*)dentry_block);
        new_dentry.file_type = *(dentry_block + FILE_TYPE);
        new_dentry.index_node_num = *(dentry_block + INODE_NUM);
        boot_data.dir_entries[dir] = new_dentry;
    }
}

/* print_file_name
 * 
 * Prints the file name corresponding to a fname passed in
 * Inputs: fname - file name to print out 
 * Outputs: None
 * Side Effects: Prints file name
 */
void print_file_name(const uint8_t* fname){
    int i;
    int file_name_length = (strlen((int8_t*)fname) > MAX_FILE_NAME_LENGTH) ? MAX_FILE_NAME_LENGTH : strlen((int8_t*)fname);
    int width = MAX_FILE_NAME_LENGTH;
    int spaces_needed = width - file_name_length;
    printf("file_name: ");
    for(i = 0; i < spaces_needed; i++) {
        printf(" ");
    }
    for(i = 0;i < file_name_length;++i){
        putc(fname[i]);
    }
}

/* print_dentry
 * 
 * Prints file type and size corresponding 
 * Inputs: dentry - pointer to dentry to print out
 * Outputs: None
 * Side Effects: Prints out meta data corresponding to the dentry 
 */
void print_dentry(dentry_t* dentry){
		printf(", ");
		printf("file_type: %d, ", dentry->file_type);
		printf("file_size: %d\n", *(boot_data.boot_addr + (dentry->index_node_num + 1) * BLOCK_SIZE_FOUR_BYTES));
}

/* read_dentry_by_name
 * 
 * Copies over dentry data for dentry with the given file name 
 * Inputs: fname - file name to match for filling dentry
 *         dentry - pointer to dentry to fill with data
 * Outputs: 0 if sucessful, -1 if not
 * Side Effects: Copies over data to dentry passed in
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry){
    /* Check if length is longer than max file name */
    if (strlen((const int8_t*)fname) > MAX_FILE_NAME_LENGTH) return -1;

    int dir;
    for(dir = 0; dir < boot_data.num_dir_entries; ++dir) {
        int8_t* curr_name = (int8_t*)boot_data.dir_entries[dir].file_name;
        int8_t* target_name = (int8_t*)fname;
        if (strncmp(target_name, curr_name, MAX_FILE_NAME_LENGTH) == 0) {
            //printf("Match found!\n");
            strcpy((int8_t*)(dentry->file_name), curr_name);
            dentry->file_type = boot_data.dir_entries[dir].file_type;
            dentry->index_node_num = boot_data.dir_entries[dir].index_node_num;
            return 0;
        }
    }
    //printf("Match not found!\n");
    //printf("%s\n", fname);
    return -1;
}

/* read_dentry_by_index
 * 
 * Copies over dentry data for dentry with the given index
 * Inputs: index - index in the dentry block in boot block
 *         dentry - pointer to dentry to fill with data
 * Outputs: 0 if sucessful, -1 if not
 * Side Effects: Copies over data to dentry passed in
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry){
    /* Return -1 if the index is out of bounds */
    if( (index < 0) || (index >= boot_data.num_dir_entries) ){
        printf("Invalid directory index!\n");
        return -1;
    }

    /* Copy contents from dir_entries array into dentry passed in */
    strcpy((int8_t*)(dentry->file_name), (int8_t*)(boot_data.dir_entries[index].file_name));
    dentry->file_type = boot_data.dir_entries[index].file_type;
    dentry->index_node_num = boot_data.dir_entries[index].index_node_num;
    return 0;
}

/* Takes the given inode and copies length bytes starting at a given
 * offset within the inode block into the passed buffer */
/* read_data
 * 
 * Copies over dentry data for dentry with the given index
 * Inputs: inode - dentry inode to read data from
 *         offset - offset within the file to read from
 *         buf - buffer to copy file contents into
 *         length - number of bytes to copy over
 * Side Effects: Copies over data to dentry passed in
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    /* Check if inode provided is valid */
    if (inode < 0 || inode >= boot_data.num_inodes) return -1;
    if (length == 0) return 0;

    /* Number of bytes read during function call */
    int32_t bytes_read = 0; 
    /* Starting address of inode block to read from */
    inode_t *inode_block = (inode_t*)(boot_data.boot_addr + ((inode + 1) * BLOCK_SIZE_FOUR_BYTES));
    /* Starting data block number to read from */
    uint32_t dblock_num = 0;
    /* Address of data block to read from */
    uint8_t *dblock_addr;
    /* Bytes to read from current block */
    uint32_t curr_block_size;

    /* Loop through each data block until we reach the length number of bytes read */
    while (dblock_num <= NUM_DATA_BLOCKS) {
        /* Check for invalid data block num */
        if (inode_block->data_blocks[dblock_num] < 0 || inode_block->data_blocks[dblock_num] >= boot_data.num_data_blocks) return -1;

        /* Address into first data block after last inode block */
        dblock_addr = (uint8_t*)((boot_data.boot_addr) + ((boot_data.num_inodes + inode_block->data_blocks[dblock_num] + 1) * BLOCK_SIZE_FOUR_BYTES));
        curr_block_size = BLOCK_SIZE;

        /* If offset is bigger than a block, go to next data block and continue loop */
        if (offset >= BLOCK_SIZE) {
            offset -= BLOCK_SIZE;
            dblock_num++;
            continue;
        }
        /* Offset first offset bytes into dblock_addr */
        else if (offset > 0) {
            dblock_addr += offset;
            curr_block_size -= offset;
            offset = 0;
        }

        /* Copy bytes from each data block into buffer */
        if (length - bytes_read < curr_block_size){ 
            memcpy(buf, dblock_addr, length - bytes_read);
            bytes_read += length - bytes_read;
        }
        else {
            memcpy(buf, dblock_addr, curr_block_size);
            bytes_read += curr_block_size;
        }

        if (bytes_read == length) return bytes_read;

        /* Iterate buffer and data block count */
        buf += curr_block_size;
        dblock_num++;
    }

    return bytes_read;
}

/* get_inode_length
 * 
 * Gets the length variable marked at the beginning of an inode
 * Inputs: fd - file descriptor containing image we want to peek
 * Outputs: length of inode block
 */
int32_t get_inode_length(int32_t fd) {
    /* Retrieve inode block struct from address indexed by boot address and inode number offset */
    inode_t *inode_block = (inode_t*)(boot_data.boot_addr + ((pcbs[curr_pid]->fd_array[fd].inode + 1) * BLOCK_SIZE_FOUR_BYTES));
    return inode_block->length;
}

/* check_file_type
 * 
 * Checks if fname defines a file of the expected type
 * Inputs: fname - file name we want to check
 *         type - file type expected by caller
 * Outputs: 0 - doesn't match file type or misc. error
 *          1 - matches file type 
 */
int32_t check_file_type(const uint8_t* fname, uint32_t type) {
    dentry_t file_temp;

    /* If fname pointer is 0 or no dentry is found, return 0 */
    if ( fname == 0 || (read_dentry_by_name(fname, &file_temp) == -1) ) return 0;
    
    /* Check file type */
    if (file_temp.file_type != type) return 0;

    return 1;
}

/* check_program_image
 * 
 * Checks if program image has correct executable format and is readable
 * Inputs: fd - file descriptor containing image we want to copy
 *         usr_prgm - location in memory to be copied to
 * Outputs: 0 - if program image is good
 *          -1 - if program is an invalid executable or other error
 */
int32_t check_program_image(int32_t fd, void* usr_prgm) {
    /* Check if user program location is valid and if fd is a file */
    if (usr_prgm == 0) return -1;

    uint8_t buf[4];

    /* Check if program image is a valid executable */
    if (file_read_nopos(fd, (void*)buf, 4) == -1) return -1;

    /* Executable magic number sequence */
    if (buf[0] != (uint8_t)0x7F) return -1;
    if (buf[1] != (uint8_t)0x45) return -1;
    if (buf[2] != (uint8_t)0x4C) return -1;
    if (buf[3] != (uint8_t)0x46) return -1;
    
    return 0;
}

/* load_program_image
 * 
 * Loads program image from filesystem into passed user program location in memory
 * Inputs: fd - file descriptor containing image we want to copy
 *         usr_prgm - location in memory to be copied to
 * Outputs: 0 - if loading was successful
 *          -1 - if loading encounters an error
 */
int32_t load_program_image(int32_t fd, void* usr_prgm) {
    /* Load program image into buffer */
    uint32_t length = get_inode_length(fd);

    int32_t bytes_read;

    if ((bytes_read = file_read_nopos(fd, usr_prgm, length)) == -1) return -1;
    
    return bytes_read;
}

/* find_program_entry
 * 
 * Finds entry point for program within the header of the executable
 * Inputs: fd - file descriptor containing image we want to copy
 * Outputs: address of entry point for program
 */
void* find_program_entry(int32_t fd) {
    /* We want to read 24-28 bytes but each 32 int takes up 4 bytes */
    uint32_t buf[(28 / 4)];

    /* Read 28 bytes and return 0 if fail */
    if (file_read_nopos(fd, (void*)buf, 28) == -1) return (void*)0;

    return (void*)(buf[(24 / 4)]);
}

