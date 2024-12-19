/* rtc.c - Functions for calling exceptions and handling
 * vim:ts=4 noexpandtab
 */

#include "rtc.h"

/* Local functions */
static uint32_t calculate_rtc_rate(uint32_t freq);

/* Defining global data */
volatile terminal_rtc_t terminal_rtc_data[TERMINAL_COUNT] = {{-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}};

/* RTC read wait flag */
volatile int rtc_read_flag = 1;
volatile int rtc_counter = 0;

/* Enable IRQ connected to RTC */
void enable_rtc_interrupt() {
    /* Enable IRQ corresponding to RTC interrupts*/
    enable_irq(RTC_IRQ);
}

/* Initialize RTC interrupts */
void rtc_init() {
    cli();
    /* Select Register B */
    outb(RTC_DISABLE_NMI + RTC_REGISTER_B, RTC_INDEX_PORT);

    /* Enable periodic interrupts in register B of RTC */
    uint8_t prev = inb(RTC_DATA_PORT);
    outb(RTC_DISABLE_NMI + RTC_REGISTER_B, RTC_INDEX_PORT);
    outb(prev | 0x40, RTC_DATA_PORT);//turns on bit 6 of register B

    sti();
}

/* rtc_update_rate
 * 
 * Changes the rate of RTC interrupts
 * Inputs: new_rate - rate to update RTC interrupts to.
 *                    This must be a power of 2 less than 1024.
 * Outputs: None
 * Side Effects: Changes the rate at which RTC interrupts are recieved
 */
void rtc_update_rate(uint32_t new_freq){
    cli();

    /* Calculate new rate based on input frequency */
    uint32_t new_rate = calculate_rtc_rate(new_freq);

    /* Select Register A */
    outb(RTC_DISABLE_NMI + RTC_REGISTER_A, RTC_INDEX_PORT);

    /* Get initial value of Register A and update rate */
    uint8_t prev = inb(RTC_DATA_PORT);
    outb(RTC_DISABLE_NMI + RTC_REGISTER_A, RTC_INDEX_PORT);
    outb((prev & 0xF0) | new_rate, RTC_DATA_PORT);//bottom 4 bits is the new rate 
    
    sti();
}

/* calculate_rtc_rate
 * 
 * Converts frequency passed in into an RTC rate
 * Inputs: freq - frequency to convert into an RTC rate
 * Outputs: rate corresponding to input frequency
 * Side Effects: None
 */
uint32_t calculate_rtc_rate(uint32_t freq){
    int temp_freq = freq;
    int new_rate = 0;
    
    /* Shift frequency to left until 32768 is reached */
    while (temp_freq != RTC_RATE_CONSTANT) {
        temp_freq <<= 1;
        new_rate++;
    }

    /* Add 1 to account for "- 1" in 32768 >> (rate - 1)*/
    return new_rate + 1;
}

/* rtc_virtualized_open
 * 
 * Initializes RTC and sets frequency to 512Hz. With the virtualized RTC, we only set the 
 * rate through registers one time. On any future writes that the user wishes to do for RTC,
 * we make use of a frequency divider and counter to give them the illusion that their program 
 * is running at the desired rate when in reality it always runs at 512Hz.
 * Inputs: None
 * Outputs: None
 * Side Effects: Initializes RTC once, additional open calls simply reset rtc data (flags, counter, rate)
 */
void rtc_virtualized_open(){
    /* Clear interrupts for critical section (terminal_active) */
    rtc_init();
    rtc_update_rate(RTC_MASTER_RATE);
    enable_rtc_interrupt();
}

/* rtc_open
 * 
 * Initializes RTC and sets frequency to 2Hz
 * Inputs: None
 * Outputs: always 0 after RTC is initialized properly
 * Side Effects: Initializes RTC
 */
int32_t rtc_open(const uint8_t *fname){
    /* Set the current process's RTC rate to the deafult and counter to 0 */
    /* Clear interrupts for critical section (terminal_active) */
    cli();
    terminal_rtc_data[terminal_active].rtc_rate = RTC_MASTER_RATE / RTC_DEFAULT_RATE;
    terminal_rtc_data[terminal_active].rtc_counter = 0;
    terminal_rtc_data[terminal_active].rtc_read_flag = 0;
    sti();

    return 0;
}

/* rtc_close
 * 
 * Does nothing (not virtualized yet)
 * Inputs: None
 * Outputs: always 0 after RTC is initialized properly
 * Side Effects: Does nothing
 */
int32_t rtc_close(int32_t fd){
    /* Set the current process's RTC as not present since we are closing it and reset counter */
    /* Clear interrupts for critical section (terminal_active) */
    cli();
    terminal_rtc_data[terminal_active].rtc_rate = -1;
    terminal_rtc_data[terminal_active].rtc_counter = 0;
    terminal_rtc_data[terminal_active].rtc_read_flag = 0;
    sti();

    return 0;
}

/* void rtc_read()
 * Inputs: None
 * Return Value: always 0 after a single interrupt has been blocked
 * Function: blocks a single RTC interrupt and returns */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
    /* Wait for RTC handler to clear flag */
    while(!terminal_rtc_data[terminal_active].rtc_read_flag){
        continue;
    }

    /* Clear interrupts for critical section (terminal_active) */
    cli();
    terminal_rtc_data[terminal_active].rtc_read_flag--;
    sti();

    return 0;
}

/* void rtc_write(int32_t fd, const void* buf, int32_t nbytes)
 * Inputs: fd - file descriptor (maybe)
           buf - pointer to 4 byte input representing new interrupt frequency
           nbytes - number of bytes to write based of buf size (maybe)
 * Return Value: always 0 after a single interrupt has been blocked
 * Function: sets a new value for the RTC interrup rate */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
    const uint32_t* freq = (const uint32_t*) buf;

    if(*freq > RTC_MAX_FREQUENCY){//new frequency is above desired max (1024 Hz)
        return -1;
    }
    if(*freq & (*freq - 1) && (*freq != 0)){//new frequency is not a power of 2
        return -1;
    }

    /* Set the RTC rate of the current process equal to the frequency passed in (virtualized) */
    /* Clear interrupts for critical section (terminal_active) */
    cli();
    terminal_rtc_data[terminal_active].rtc_rate = RTC_MASTER_RATE / *freq;
    terminal_rtc_data[terminal_active].rtc_counter = 0;
    terminal_rtc_data[terminal_active].rtc_read_flag = 0;
    sti();
    
    return 0;
}

/* rtc_test
 * 
 * Helps test the rtc read/write functions
 * Inputs: None
 * Outputs: None
 * Side Effects: Cycles through different RTC frequencies and reads/writes 
 *               to print characters at different speeds to the screen.
 */
void rtc_test(){
    enable_rtc_interrupt();
    clear();
    //rtc_read_counter = 0;
    uint32_t freq = 2;
    uint32_t read_buf;
    while(freq <= RTC_MAX_FREQUENCY){
        int i;
        for(i = 0;i < freq;++i){
            rtc_read(0, &read_buf, 4);
        }
        clear();
        rtc_write(0, &freq, 4);
        freq *= 2;
    }
    disable_irq(RTC_IRQ);
    clear();
    printf("RTC read/write test complete!\n");
}
