/* timer.c - PIT (Programmable Interval Timer) implementation */
#include "timer.h"
#include "idt.h"
#include "io.h"
#include "serial.h"
#include "scheduler.h"
#include "helper.h"

/* Timer tick counter */
static volatile uint32_t timer_ticks = 0;
static volatile uint32_t context_switches_seen = 0;

/* PIT oscillator frequency: 1.193182 MHz */
#define PIT_FREQUENCY 1193180

/* Timer IRQ handler - called on every timer tick */
static void timer_handler(struct interrupt_registers *regs) {
    (void)regs;  /* Unused parameter */
    
    /* Increment tick counter */
    timer_ticks++;
    
    /* Debug: Print every single tick when we're just starting */
    if (timer_ticks <= 5) {
        serial_puts("[TIMER] Tick ");
        char num_str[12];
        int_to_str(timer_ticks, num_str);
        serial_puts(num_str);
        serial_puts("\n");
    }
    
    /* Debug: After first context switch, print frequently to see if interrupts still fire */
    if (context_switches_seen > 0 && timer_ticks % 10 == 0) {
        serial_puts("[TIMER_AFTER_CTX] Tick ");
        char num_str[12];
        int_to_str(timer_ticks, num_str);
        serial_puts(num_str);
        serial_puts("\n");
    }
    
    /* Debug: print tick every 100 ticks initially */
    if (context_switches_seen == 0 && timer_ticks % 100 == 0) {
        serial_puts("[TIMER] Tick ");
        char num_str[12];
        int_to_str(timer_ticks, num_str);
        serial_puts(num_str);
        serial_puts("\n");
    }

    /* Check if context switch happened */
    extern uint32_t total_context_switches;
    if (total_context_switches > context_switches_seen) {
        context_switches_seen = total_context_switches;
        serial_puts("[TIMER] Detected context switch!\n");
    }

    /* Call scheduler to potentially switch processes */
    scheduler_tick();
}

/* Initialize the PIT timer */
void timer_init(void) {
    /* Calculate divisor for desired frequency */
    uint32_t divisor = PIT_FREQUENCY / TIMER_FREQ;

    /* Install timer handler on IRQ0 */
    irq_install_handler(0, timer_handler);

    /* Send command byte to PIT
     * 0x36 = 0011 0110 binary
     * - Channel 0
     * - Access mode: lobyte/hibyte
     * - Operating mode: 3 (square wave generator)
     * - Binary mode
     */
    outb(0x43, 0x36);

    /* Send divisor (low byte, then high byte) */
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));

    timer_ticks = 0;

    serial_puts("PIT timer initialized.\n");
    serial_puts("Frequency: ");
    char num_str[12];
    int_to_str(TIMER_FREQ, num_str);
    serial_puts(num_str);
    serial_puts(" Hz (");
    int_to_str(1000 / TIMER_FREQ, num_str);
    serial_puts(num_str);
    serial_puts(" ms per tick)\n");
}

/* Get the number of timer ticks since boot */
uint32_t timer_get_ticks(void) {
    return timer_ticks;
}

/* Sleep for a specified number of timer ticks */
void timer_wait(uint32_t ticks) {
    uint32_t target = timer_ticks + ticks;
    while (timer_ticks < target) {
        __asm__ volatile("hlt");  /* Halt CPU until next interrupt */
    }
}
