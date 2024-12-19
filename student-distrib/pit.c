/* pit.c - Functions for PIT initiailization/interfacing
 * vim:ts=4 noexpandtab
 */

#include "pit.h"

void enable_pit_interrupt(){
    enable_irq(PIT_IRQ);
}

void pit_init(){
    cli();

    int divisor = PIT_DIVISOR(20); /* Calculate our divisor */
    outb(0x36, PIT_COMMAND_REGISTER); /* Set our command byte 0x36 */
    outb(divisor & 0xFF, PIT_CHANNEL_0); /* Set low byte of divisor */
    outb(divisor >> 8, PIT_CHANNEL_0); /* Set high byte of divisor */

    sti();
}
