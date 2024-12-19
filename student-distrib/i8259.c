/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* i8259_init
 * Description: Initialize the 8259 PIC by writing the proper bytes to the slave and master PIC.
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 
*/
void i8259_init(void) {
    /* disable all interrupts on both PIC's*/
    // outb(0xFF, MASTER_8259_DATA_PORT);
    // outb(0xFF, SLAVE_8259_DATA_PORT);

    /* master PIC initialization */
    outb(ICW1, MASTER_8259_COMMAND_PORT);
    outb(ICW2_MASTER, MASTER_8259_DATA_PORT);
    outb(ICW3_MASTER, MASTER_8259_DATA_PORT);
    outb(ICW4, MASTER_8259_DATA_PORT);

    /* slave PIC initialization */
    outb(ICW1, SLAVE_8259_COMMAND_PORT);
    outb(ICW2_SLAVE, SLAVE_8259_DATA_PORT);
    outb(ICW3_SLAVE, SLAVE_8259_DATA_PORT);
    outb(ICW4, SLAVE_8259_DATA_PORT);

    /* restore IRQ masks on both PIC's */
    master_mask = 0xFB; // enable IRQ2 on master since slave is connected to it
    slave_mask = 0xFF;
    outb(master_mask, MASTER_8259_DATA_PORT);
    outb(slave_mask, SLAVE_8259_DATA_PORT);

    //printf("PIC Initialized!\n");
}
/* enable_irq
 * Description: Enable the specified IRQ by unmasking the corresponding bit in the PIC
 * Inputs: irq_num - the IRQ number to enable
 * Outputs: None
 
*/
void enable_irq(uint32_t irq_num) {
    if(irq_num >= 8){//enable IRQ on slave PIC (IRQ's 8-15)
        uint8_t mask = ~(1 << (irq_num - 8));
        outb(slave_mask & mask, SLAVE_8259_DATA_PORT);
        /* Update the current slave mask to mantain old irq state for slave PIC */
        slave_mask = slave_mask & mask;
        // printf("IRQ%d Enabled!\n", irq_num);
        // printf("Master mask: %x\n", master_mask);
        // printf("Slave mask: %x\n", slave_mask);
    }
    else{//enable IRQ on master PIC (IRQ's 0-7)
        uint8_t mask = ~(1 << irq_num);
        outb(master_mask & mask, MASTER_8259_DATA_PORT);
        /* Update the current master mask to mantain old irq state for master PIC */
        master_mask = master_mask & mask;
        // printf("IRQ%d Enabled!\n", irq_num);
        // printf("Master mask: %x\n", master_mask);
        // printf("Slave mask: %x\n", slave_mask);
    }
}
/* disable_irq
 * Description: Disable the specified IRQ by masking the corresponding bit in the PIC
 * Inputs: irq_num - the IRQ number to disable
 * Outputs: None
 
*/
void disable_irq(uint32_t irq_num) {
    if(irq_num >= 8){//disable IRQ on slave PIC (IRQ's 8-15)
        uint8_t mask = (1 << (irq_num - 8));
        outb(slave_mask | mask, SLAVE_8259_DATA_PORT);
    }
    else{//disable IRQ on master PIC (IRQ's 0-7)
        uint8_t mask = (1 << irq_num);
        outb(master_mask | mask, MASTER_8259_DATA_PORT);
    }
}
/* send_eoi
 * Description: Send end-of-interrupt signal for the specified IRQ
 * Inputs: irq_num - the IRQ number to send EOI to
 * Outputs: None
 

*/
void send_eoi(uint32_t irq_num) {
    if(irq_num >= 8){//if IRQ is between 7 and 15, send EOI to secondary PIC
        outb(0x02 | EOI, MASTER_8259_COMMAND_PORT);
        outb((irq_num - 8) | EOI, SLAVE_8259_COMMAND_PORT);
    }
    else{
        outb(irq_num | EOI, MASTER_8259_COMMAND_PORT);
    }
    //printf("Sending EOI!\n");
}
