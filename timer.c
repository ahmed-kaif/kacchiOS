/* timer.c - Programmable Interval Timer implementation */
#include "timer.h"
#include "idt.h"
#include "serial.h"
#include "scheduler.h"
#include "helper.h"
#include "io.h"

/* Tick counter */
static volatile uint32_t timer_ticks = 0;

/* External interrupt handler from assembly */
extern void irq0_handler(void);

/* Timer interrupt handler (called from assembly) */
void timer_interrupt_handler(void)
{
    timer_ticks++;

    /* Don't call scheduler from interrupt - causes issues with context switch */
    /* TODO: Implement proper interrupt-safe context switching */
    /* scheduler_schedule(); */
}

/* Initialize timer with specified frequency */
void timer_init(uint32_t frequency)
{
    uint32_t divisor;
    uint8_t low, high;

    /* Calculate divisor */
    divisor = PIT_FREQUENCY / frequency;

    low = (uint8_t)(divisor & 0xFF);
    high = (uint8_t)((divisor >> 8) & 0xFF);

    /* Set up timer interrupt handler (IRQ 0 = interrupt 32) */
    idt_set_gate(32, (uint32_t)irq0_handler, 0x08, 0x8E);

    /* Configure PIT */
    /* Command: channel 0, lobyte/hibyte, rate generator */
    outb(PIT_COMMAND, 0x36);

    /* Set frequency divisor */
    outb(PIT_CHANNEL0, low);
    outb(PIT_CHANNEL0, high);

    serial_puts("Timer initialized at ");
    char freq_str[12];
    int_to_str(frequency, freq_str);
    serial_puts(freq_str);
    serial_puts(" Hz.\n");
}

/* Get current tick count */
uint32_t timer_get_ticks(void)
{
    return timer_ticks;
}
