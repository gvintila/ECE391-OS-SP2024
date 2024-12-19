/* rtc.h - Defines functions used to initialize RTC interrupts
 * vim:ts=4 noexpandtab
 */

#ifndef _RTC_H
#define _RTC_H

#include "x86_inter.h"
#include "lib.h"
#include "scheduler.h"

/* Ports used for RTC and CMOS */
#define RTC_INDEX_PORT    0x70
#define RTC_DATA_PORT     0x71
#define RTC_DISABLE_NMI   0x80
#define RTC_REGISTER_A    0x0A
#define RTC_REGISTER_B    0x0B
#define RTC_REGISTER_C    0x0C

/* RTC rate/frequency data */
#define RTC_DEFAULT_RATE     2
#define RTC_MAX_FREQUENCY    1024
#define RTC_RATE_CONSTANT    32768

/* This will always be the RTC rate and we will virtualize each terminals RTC rate */
#define RTC_MASTER_RATE      512

/* Fuck C */
#define TERMINAL_COUNT      3

#ifndef ASM

/* Store RTC information for each terminal */
typedef struct terminal_rtc{
    int32_t rtc_rate; //stores each terminal's RTC rate (-1 signifies rtc_open)
    int32_t rtc_counter; //stores the counter for this terminal's RTC 
    uint32_t rtc_read_flag; //flag used for rtc_read and to virtualize each terminal's RTC rate
} terminal_rtc_t;

/* Initialize array of terminal rtc structs */
extern volatile terminal_rtc_t terminal_rtc_data[TERMINAL_COUNT];

/* Flag for RTC to debug */
extern volatile int rtc_read_flag;
extern volatile int rtc_counter;

/* Initialize RTC interrupts */
extern void rtc_init();

/* Enable IRQ line connected to RTC */
extern void enable_rtc_interrupt();

/* Changes RTC interrupt rate */
extern void rtc_update_rate(uint32_t new_freq);

/* Initializes RTC and sets the interrupts rate to a default value */
extern int32_t rtc_open(const uint8_t *fname);

/* Does nothing for now */
extern int32_t rtc_close(int32_t fd);

/* Initializes RTC and sets the interrupts rate to a default value */
extern int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

/* Changes the RTC interrupt rate based on a 4-byte input */
extern int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

/* Initializes RTC to master frequency once while the rest of the implementation is virtualized */
extern void rtc_virtualized_open();

/* Helps test rtc read and write functions by chaning the interrupt rate */
extern void rtc_test();

#endif /* _RTC_H */

#endif /* ASM */
