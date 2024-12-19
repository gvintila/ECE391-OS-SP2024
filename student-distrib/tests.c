#include "tests.h"
#include "x86_desc.h"
#include "x86_inter.h"
#include "lib.h"
#include "types.h"
#include "filesystem.h"
#include "terminal.h"
#include "syscall.h"
#include "pid.h"
#include "syscall.h"
#include "pid.h"

#define PASS 1
#define FAIL 0

#define MAX_FILE_SIZE	40000

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* Exception # Tests
 * 
 * Asserts that exception # gets properly raised
 * Inputs: None
 * Outputs: Prints which exception was raised
 * Side Effects: Freezes OS
 * Coverage: Load IDT, Exception Handling
 * Files: x86_excep.h/S
 */
void excep0_test(){
	TEST_HEADER;

	int i;
	int zero = 0;
	i = 3 / zero;
	/* Exception should be raised here after incorrect division occurs */
}

void excep1_test(){ TEST_HEADER; asm ("int $1");
	/* Exception should be raised here after software interrupt */
}

/* We can't use inline assembly because each operand must be an immediate value... */
void excep2_test(){ TEST_HEADER; asm ("int $2"); }
void excep3_test(){ TEST_HEADER; asm ("int $3"); }
void excep4_test(){ TEST_HEADER; asm ("int $4"); }
void excep5_test(){ TEST_HEADER; asm ("int $5"); }
void excep6_test(){ TEST_HEADER; asm ("int $6"); }
void excep7_test(){ TEST_HEADER; asm ("int $7"); }
void excep8_test(){ TEST_HEADER; asm ("int $8"); }
void excep9_test(){ TEST_HEADER; asm ("int $9"); }
void excep10_test(){ TEST_HEADER; asm ("int $10"); }
void excep11_test(){ TEST_HEADER; asm ("int $11"); }
void excep12_test(){ TEST_HEADER; asm ("int $12"); }
void excep13_test(){ TEST_HEADER; asm ("int $13"); }
void excep14_test(){ TEST_HEADER; asm ("int $14"); }
void excep16_test(){ TEST_HEADER; asm ("int $16"); }
void excep17_test(){ TEST_HEADER; asm ("int $17"); }
void excep18_test(){ TEST_HEADER; asm ("int $18"); }
void excep19_test(){ TEST_HEADER; asm ("int $19"); }

/* System call test */
void sys_call_test(){ TEST_HEADER; asm ("int $0x80"); }

/* Page dereference tests
 * 
 * Checks dereferencing valid and invalid sections
 * Inputs: None
 * Outputs: Prints dereferenced value or exception
 * Side Effects: Freezes OS
 * Coverage: Paging
 * Files: paging.c/.h
 */
int page_deref_valid_test(){
	TEST_HEADER;

	int *mem = (int *)VIDEO;
	int result = PASS;

	printf("Dereferencing valid memory at video memory: %d\n", *mem);

	return result;
}
void page_deref_invalid_test(){
	TEST_HEADER;

	int *mem = (int *)(6 * BIG_PAGE_SIZE);

	printf("Dereferencing invalid memory at %d:", mem);
	printf("%d\n", *mem);
}



/* Checkpoint 2 tests */

/* terminal_read_error_test
 * 
 * Checks if terminal_read returns 0 when memory address is 0
 * Inputs: None
 * Outputs: Prints number of bytes copied
 
*/
void terminal_read_error_test() {
	
	if (!terminal_read(0, 0, 12)) {
		printf("Num bytes copied: %d\n", terminal_read(0, 0, 45));
	}

}
/* terminal_read_diff_size_test
 * 
 * Checks for handling of different buffer sizes
 * Inputs: None
 * Outputs: Prints number of bytes copied
 

*/
void terminal_read_diff_size_test() {
	uint8_t test_buf[45];
	
	printf("Num bytes copied: %d\n", terminal_read(0, test_buf, 45));
	terminal_write(0, test_buf, 45);
}
/* terminal_write_test
 * 
 * Checks if terminal_write can write 10000 bytes (requires scrolling)
 * Inputs: None
 * Outputs: None
 
*/
void terminal_write_test() {
    uint8_t test_buf[10000];
    int  i;
    for (i = 0; i < 10000; i++) {
        test_buf[i] = 'L';
    }    
    terminal_write(0, test_buf, 10000);
}
/* terminal_buf_size_greater
 * 
 * Checks if terminal_read can handle a buffer size greater than 128
 * Inputs: None
 * Outputs: Prints number of bytes copied
 

*/
void terminal_buf_size_greater() {
	uint8_t test_buf[130];

	printf("Num bytes copied: %d\n", terminal_read(0, test_buf, 130));

	terminal_write(0, test_buf, 130);
}
/* Terminal Test Regular
 * 
 * Checks overall functionality of the  terminal 
 * Inputs: None
 * Outputs: Reads from keyboard buffer using terminal read then populates the buffer sent in, then it uses that buffer 
 *          to print as a parameter for terminal write to write that buffer to the buffer.
 * Side Effects: 
 * Coverage: Terminal
 * Files: kboard.c, kboard.h, x86_inter.c
 */
void terminal_test_regular() {
	uint8_t test_buf[128];

	while(1) {
		printf("Num Bytes copied %d\n", terminal_read(0, test_buf, 128));
		printf("Num Bytes written %d\n", terminal_write(1, test_buf, 128));
	}	
}

/* Terminal Test Write Size Greater
 * 
 * Checks overall functionality of the  terminal 
 * Inputs: None
 * Outputs: Prints out number of bytes copied as well as printed. They should match, even though the user
 * 			sent in a number that is greater 128 for the terminal_write
 * Side Effects: 
 * Coverage: Terminal
 * Files: kboard.c, kboard.h, x86_inter.c
 */
void terminal_write_size_greater() {
    uint8_t test_buf[128];

    printf("Num Bytes copied %d\n", terminal_read(0, test_buf, 128));
    printf("Num Bytes printed: %d\n", terminal_write(0, test_buf, 145));
}

void terminal_write_test_size() {
    TEST_HEADER;
    uint8_t test_buf[128];
    int i;
    for (i = 0; i < 127; i++) {
        test_buf[i] = 'A';
    }
    test_buf[127] = '\n'; // null terminate the string
    printf("TEST: Writing 150 bytes to terminal, even though buf_size is 128...\n");
    if ((terminal_write(0, test_buf, 150)) != 150) {
        printf("Error: Trying to write # bytes greater than buffer size. Printed buf_size bytes instead\n");
    
    }; // trying to write more than 128 bytes (BUF_SIZE)
}

/* Terminal Test Other
 * 
 * Checks overall functionality of the  terminal 
 * Inputs: None
 * Outputs: Series of tests for the user to run and see different functionality
 * Side Effects: 
 * Coverage: Terminal
 * Files: kboard.c, kboard.h, x86_inter.c
 */
void terminal_test_other() {


    while(1) {
        printf("Write any thing to keyboard buffer and press Enter for each test\n");

        printf("Test 1: Buf size is less than 128\n");
        terminal_read_diff_size_test();
        putc('\n');

        printf("Test 2: Passing in 0 as memory addr for buf\n");
        terminal_read_error_test();

        printf("Test 3: Buf size is greater than 128\n");
        terminal_buf_size_greater();

        printf("Test 4: Terminal write test where num_bytes is greater than size of buf\n");
        terminal_write_size_greater();
   
    }
}

/* File system dentry test (Repurposed for CP3)
 * 
 * Prints information regarding all dentries in the current file system
 * Inputs: None
 * Outputs: Prints file name, file type, and file size for each dentry
 * Side Effects: Prints to screen after clearing it
 * Coverage: File system
 * Files: filesystem.c/.h
 */
void list_all_files_test(){
    int dir;
	int fd;
	const uint8_t* dir_name = (const uint8_t*)".";
	int8_t fname[MAX_FILE_NAME_LENGTH]; 
	printf("Opening directory: %d\n\n", fd = open(dir_name));

    for(dir = 0;dir < boot_data.num_dir_entries;++dir){
		dentry_t curr_dentry;
		read_dentry_by_index(dir, &curr_dentry);
		read(fd, fname, MAX_FILE_NAME_LENGTH);
		print_file_name((uint8_t*)(fname));
		print_dentry(&curr_dentry);
	}

	printf("\nAdditional directory reads: %d\n", read(fd, fname, MAX_FILE_NAME_LENGTH));
	printf("Attempting directory write: %d\n", write(fd, fname, MAX_FILE_NAME_LENGTH));
	printf("Closing directory: %d\n", close(fd));
	printf("Attempting read after close: %d\n", read(fd, fname, MAX_FILE_NAME_LENGTH));
}

/* Single file test (Repurposed for CP3)
 * 
 * Prints contents of a single file in the file system
 * Inputs: fname - name of the file whose contents will be print out
 * Outputs: Prints entire file to the console
 * Side Effects: Prints to screen after clearing it
 * Coverage: File system
 * Files: filesystem.c/.h
 */
void print_file_contents(const uint8_t* fname, int file_size){
	int i;
	int fd;
	int bytes_read = 0;
	uint8_t buf[MAX_FILE_SIZE];

	printf("Opening file: %d\n", fd = open(fname));
	bytes_read = read(fd, buf, file_size);
	
	for(i = 0;i < bytes_read; ++i){
		if((char)(buf[i]) != '\0'){
			putc(buf[i]);
		}
	}

	printf("\nBytes read: %d\n", bytes_read);
	print_file_name(fname);
	printf("\nAttempting file write: %d\n", write(fd, buf, file_size));
	printf("Closing file: %d\n", close(fd));
	printf("Attempting read after close: %d\n", read(fd, buf, file_size));
}

/* Checkpoint 3 tests */

/* Initial Sys Call Test
 * 
 * Calls each system call and just prints out the arguments
 * Inputs: None
 * Outputs: Prints arguments of each system call
 * Side Effects: Prints to screen
 * Coverage: System calls
 * Files: syscall.c/h and x86_interrupts.S
 */
void sys_call_handler_test(){
	// uint8_t status = 0;
	// uint8_t command = 1;
	// int32_t fd = 2;
	// uint8_t buf[10];
	// int32_t nbytes = 20;
	// const uint8_t* filename = "file.txt";
	// uint8_t** screen_start;
	// int32_t signum = 45;
	// void* handler_address = 10;
	// halt(status);
	// execute(command);
	// read(fd, buf, nbytes);
	// write(fd, buf, nbytes);
	// open(filename);
	// close(fd); 
	// getargs(buf, nbytes);
	// vidmap(screen_start);
	// set_handler(signum, handler_address);
	// sigreturn();
}

/* Sys Call Open/Read/Write/Close Test
 * 
 * Calls the four system calls to check if responding operations on them
 * work as expected, and keeps track of how the fds are being stored
 * Inputs: None
 * Outputs: Varies
 * Side Effects: Prints to screen
 * Coverage: System calls
 */
void sys_call_orwc_test(){
	/* Repurposed CP2 Tests */

	/* File system tests */
	//list_all_files_test();

	/* Small file print test*/
	//print_file_contents((const uint8_t*)("frame0.txt"), MAX_FILE_SIZE);
	/* Large file print test */
	//print_file_contents((const uint8_t*)("verylargetextwithverylongname.txt"), MAX_FILE_SIZE);
	/* Executable file print test */
	//print_file_contents((const uint8_t*)("ls"), MAX_FILE_SIZE);

	/* Screen will scroll so comment whatever tests you don't want */

	/* Testing garbage values */
	printf("Opening bad file: %d\n", open((uint8_t*)"badfile.bad"));
	printf("Closing bad file: %d\n", close(10));
	printf("Writing bad file: %d\n", write(10, (void*)0, 0));
	printf("Reading bad file: %d\n", read(10, (void*)0, 0));

	/* Attempting to close stdin and stdout */
	printf("Attempting to close stdout: %d\n", close(1));
	printf("Attempting to close stdin: %d\n", close(0));

	/* Testing multiple opens at once */
	printf("Opening frame0.txt: %d\n", open((const uint8_t*)("frame0.txt")));
	printf("Opening frame0.txt: %d\n", open((const uint8_t*)("frame0.txt")));
	printf("Opening frame0.txt: %d\n", open((const uint8_t*)("frame0.txt")));
	printf("Opening frame0.txt: %d\n", open((const uint8_t*)("frame0.txt")));
	printf("Opening frame0.txt: %d\n", open((const uint8_t*)("frame0.txt")));
	printf("Opening frame0.txt: %d\n", open((const uint8_t*)("frame0.txt")));
	printf("Opening frame0.txt: %d\n\n", open((const uint8_t*)("frame0.txt")));

	/* Testing closes */
	printf("Closing fd 7: %d\n", close(7));
	printf("Closing fd 3: %d\n", close(3));
	printf("Closing fd 9: %d\n\n", close(9));

	/* Testing opens after closes */
	printf("Opening frame0.txt: %d\n", open((const uint8_t*)("frame0.txt")));
	printf("Opening frame0.txt: %d\n", open((const uint8_t*)("frame0.txt")));
	printf("Opening frame0.txt: %d\n", open((const uint8_t*)("frame0.txt")));

	/* Testing reading from a random fd */
	printf("Closing fd 3: %d\n\n", close(3));
	print_file_contents((const uint8_t*)("frame0.txt"), 10);
}

/* Sys Call Execute Test
 * 
 * Calls the execute sys call and checks to see 
 * if all the setup is loading correctly 
 * Inputs: None
 * Outputs: Varies
 * Side Effects: Prints to screen
 * Coverage: System calls
 */
void sys_call_exec_test() {
	/* I just GDB through this to make sure all the values are getting set right */
	printf("Executing shell: %d\n\n", execute((const uint8_t*)("shell")));
	printf("Executing garbage: %d\n\n", execute((const uint8_t*)("garbage")));
	printf("Executing frame0.txt: %d\n\n", execute((const uint8_t*)("frame0.txt")));
	printf("Executing ls: %d\n\n", execute((const uint8_t*)("ls")));
}

/* Syscall read and write test
 * 
 * Calls the read and write syscalls with text from the terminal.
 * Inputs: None
 * Outputs: Varies
 * Side Effects: Prints to screen
 * Coverage: System calls
 
*/
void syscall_terminal_read_test(){
uint8_t test_buf[128];
	terminal_open((const uint8_t*)"dummy");
	while(1) {
		printf("Num Bytes copied %d\n", read(0, test_buf, 128));
		
		printf("Num Bytes written %d\n", write(1, test_buf, 128));
	}	
}

/* Sys Call Read Test
 * 
 * Calls read and expects correct file_pos saving response
 * Inputs: None
 * Outputs: Varies
 * Side Effects: Prints to screen
 * Coverage: System calls
 */
void sys_call_read_test(){
	int i;
	int fd;
	int bytes_read = 0;
	uint8_t buf[MAX_FILE_SIZE];

	int32_t nbytes1 = 300;
	int32_t nbytes2 = 100;

	printf("Reading first run!\n");
	fd = open((const uint8_t*)("frame0.txt"));
	bytes_read = read(fd, buf, nbytes1);
	
	for(i = 0;i < bytes_read; ++i){
		if((char)(buf[i]) != '\0'){
			putc(buf[i]);
		}
	}

	printf("Reading second run!\n");
	bytes_read = read(fd, buf, nbytes2);
	for(i = 0;i < bytes_read; ++i){
		if((char)(buf[i]) != '\0'){
			putc(buf[i]);
		}
	}
}

/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	clear();

	/* Checkpoint 1 Tests */

	// TEST_OUTPUT("idt_test", idt_test());
	
	/* Individual exception tests */
	//sys_call_test();

	/* Test system call interrupt */
	//sys_call_test();
	
	/* Test for each exception individually (excep#_test) */
	//excep6_test();

	/* Paging tests */
	// TEST_OUTPUT("page_deref_valid_test", page_deref_valid_test());
	//page_deref_invalid_test();
	
	/* Checkpoint 2 Tests */

	/* Terminal tests*/	
	terminal_test_regular();
	//terminal_test_other();
	//terminal_write_test_size();

	/* File system tests */
	//list_all_files_test();

	/* Small file print test*/
	//print_file_contents((const uint8_t*)("frame0.txt"), MAX_FILE_SIZE);
	/* Large file print test */
	//print_file_contents((const uint8_t*)("verylargetextwithverylongname.txt"), MAX_FILE_SIZE);
	/* Executable file print test */
	//print_file_contents((const uint8_t*)("ls"), MAX_FILE_SIZE);

	/* RTC test */
    //rtc_test();

	/* Checkpoint 3 Tests */

	/* Syscall Tests */

	// sys_call_handler_test();
	// sys_call_test();

	/* adriel's test*/
	//halt((uint8_t)(8));

	/* Open/Read/Write/Close Test */
	//sys_call_orwc_test();
	//printf("Opening frame0.txt: %d\n", open((const uint8_t*)("frame0.txt")));

	/* Read Test */
	//sys_call_read_test();

	/* Tests for terminal_read, terminal_open using syscalls*/

	/* Execute Test */
	//sys_call_exec_test();
	// init_pcb(0);

}
