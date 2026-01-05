/* kernel.c - Main kernel with process manager and scheduler */
#include "memory.h"
#include "serial.h"
#include "string.h"
#include "types.h"
#include "process.h"
#include "scheduler.h"
#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include "helper.h"
#include <string.h>
#define MAX_INPUT 128

/* Example process 1: Counter process */
void process_counter(void)
{
  /* Ensure interrupts are enabled */
  __asm__ volatile("sti");
  
  int count = 0;
  while (1)
  {
    serial_puts("[Counter] Count: ");
    char num_str[12];
    int_to_str(count++, num_str);
    serial_puts(num_str);
    serial_puts("\n");

    /* Busy wait to simulate work */
    int i;
    for (i = 0; i < 50; i++)
    {
      __asm__ volatile("nop");
    }
  }
}

/* Example process 2: Idle process */
void process_idle(void)
{  
  /* Ensure interrupts are enabled */
  __asm__ volatile("sti");
  
  serial_puts("[Idle] Interrupts enabled via STI\n");
  
  /* Check if IF flag is actually set */
  uint32_t eflags;
  __asm__ volatile("pushfl; popl %0" : "=r"(eflags));
  serial_puts("[Idle] EFLAGS = 0x");
  char buf[12];
  int_to_hex(eflags, buf);
  serial_puts(buf);
  serial_puts(", IF bit = ");
  serial_puts((eflags & 0x200) ? "1\n" : "0\n");
  
  /* Try triggering a software interrupt to test IDT */
  serial_puts("[Idle] Testing software interrupt...\n");
  __asm__ volatile("int $0x80");
  serial_puts("[Idle] Software interrupt returned!\n");
  
  /* Check PIC masks */
  uint8_t mask1, mask2;
  __asm__ volatile("inb $0x21, %0" : "=a"(mask1));
  __asm__ volatile("inb $0xA1, %0" : "=a"(mask2));
  serial_puts("[Idle] PIC masks: Master=0x");
  int_to_hex(mask1, buf);
  serial_puts(buf);
  serial_puts(", Slave=0x");
  int_to_hex(mask2, buf);
  serial_puts(buf);
  serial_puts("\n");
  
  /* Test if interrupts are actually working */
  uint32_t start_ticks = timer_get_ticks();
  serial_puts("[Idle] Waiting for interrupt...\n");
  serial_puts("[Idle] Current ticks = ");
  int_to_str(start_ticks, buf);
  serial_puts(buf);
  serial_puts("\n");
  
  /* Try sending EOI to master PIC */
  serial_puts("[Idle] Sending EOI to PIC...\n");
  __asm__ volatile("outb %0, $0x20" : : "a"((uint8_t)0x20));
  
  /* Wait a bit and check again */
  volatile uint32_t delay;
  for (delay = 0; delay < 10000000; delay++);
  
  uint32_t after_delay = timer_get_ticks();
  serial_puts("[Idle] After delay, ticks = ");
  int_to_str(after_delay, buf);
  serial_puts(buf);
  serial_puts("\n");
  
  uint32_t loop_count = 0;
  while (timer_get_ticks() == start_ticks) {
    loop_count++;
    if (loop_count % 100000000 == 0) {
      serial_puts("[Idle] Still waiting, loop count = ");
      int_to_str(loop_count / 100000000, buf);
      serial_puts(buf);
      serial_puts("00M, ticks = ");
      int_to_str(timer_get_ticks(), buf);
      serial_puts(buf);
      serial_puts("\n");
    }
  }
  
  serial_puts("[Idle] Timer interrupt fired!\n");
  
  while (1)
  {
    serial_puts("[Idle] Running...\n");
    
    /* Halt until next interrupt */
    int i;
    for (i = 0; i < 50; i++)
    {
      __asm__ volatile("nop");
    }
  }
}

/* Example process 3: Worker process */
void process_worker(void)
{
  /* Ensure interrupts are enabled */
  __asm__ volatile("sti");
  
  int work_units = 0;
  while (1)
  {
    serial_puts("[Worker] Work unit: ");
    char num_str[12];
    int_to_str(work_units++, num_str);
    serial_puts(num_str);
    serial_puts("\n");

    /* Simulate work */
    // int i;
    // for (i = 0; i < 50; i++)
    // {
    //   __asm__ volatile("nop");
    // }
  }
}

void kmain(void)
{
  char input[MAX_INPUT];
  int pos = 0;

  /* Initialize hardware */
  serial_init();
  
  /* Initialize GDT (Global Descriptor Table) */
  gdt_init();
  
  /* Initialize IDT (Interrupt Descriptor Table) */
  idt_init();
  
  /* Initialize PIT timer */
  timer_init();
  
  /* Initialize memory manager */
  memory_init();
  
  /* Initialize process manager */
  process_init();
  
  /* Initialize scheduler */
  scheduler_init();

  /* memory test */
  char *test_str = (char *)kmalloc(20);
  strcpy(test_str, "Memory init success");

  /* Print welcome message */
  serial_puts("\n");
  serial_puts("========================================\n");
  serial_puts("    kacchiOS - Minimal Baremetal OS\n");
  serial_puts("========================================\n");
  serial_puts("Hello from kacchiOS!\n");
  serial_puts("Process Manager & Scheduler Demo\n\n");

  serial_puts("Memory Test: ");
  serial_puts(test_str);
  serial_puts("\n\n");

  serial_puts("Available commands:\n");
  serial_puts("  ps       - List all processes\n");
  serial_puts("  start    - Start the scheduler\n");
  serial_puts("  stats    - Show scheduler statistics\n");
  serial_puts("  mem      - Show memory statistics\n");
  serial_puts("  help     - Show this help message\n");
  serial_puts("  exit     - Halt the system\n");
  serial_puts("\n");

  /* Create example processes (currently for demonstration) */
  serial_puts("Creating demo processes...\n");
  process_create("idle", process_idle, PRIORITY_LOW);
  process_create("worker1", process_worker, PRIORITY_LOW);
  process_create("counter", process_counter, PRIORITY_LOW);
  serial_puts("\n");

  /* Auto-start scheduler for testing */
  serial_puts("Auto-starting scheduler...\n");
  serial_puts("[DEBUG] About to call scheduler_start()\n");
  scheduler_start();
  /* scheduler_start() doesn't return - it jumps to first process */
  serial_puts("[DEBUG] ERROR: Returned from scheduler_start()!\n");

  /* Main loop - the "null process" (should not be reached if scheduler starts) */
  while (1)
  {
    serial_puts("kacchiOS> ");
    pos = 0;

    /* Read input line */
    while (1)
    {
      char c = serial_getc();

      /* Handle Enter key */
      if (c == '\r' || c == '\n')
      {
        input[pos] = '\0';
        serial_puts("\n");
        break;
      }
      /* Handle Backspace */
      else if ((c == '\b' || c == 0x7F) && pos > 0)
      {
        pos--;
        serial_puts("\b \b"); /* Erase character on screen */
      }
      /* Handle normal characters */
      else if (c >= 32 && c < 127 && pos < MAX_INPUT - 1)
      {
        input[pos++] = c;
        serial_putc(c); /* Echo character */
      }
    }

    /* Echo back the input */
    if (pos > 0)
    {
      /* Process commands */
      if (strcmp(input, "ps") == 0)
      {
        process_list();
      }
      else if (strcmp(input, "start") == 0)
      {
        scheduler_start();
      }
      else if (strcmp(input, "stats") == 0)
      {
        scheduler_stats();
      }
      else if (strcmp(input, "mem") == 0)
      {
        memory_dump_stats();
      }
      else if (strcmp(input, "help") == 0)
      {
        serial_puts("Available commands:\n");
        serial_puts("  ps       - List all processes\n");
        serial_puts("  start    - Start the scheduler\n");
        serial_puts("  stats    - Show scheduler statistics\n");
        serial_puts("  mem      - Show memory statistics\n");
        serial_puts("  help     - Show this help message\n");
        serial_puts("  exit     - Halt the system\n");
        serial_puts("\n");
      }
      else if (strcmp(input, "exit") == 0)
      {
        serial_puts("Halting system...\n");
        break;
      }
      else
      {
        serial_puts("You typed: ");
        serial_puts(input);
        serial_puts("\n");
        serial_puts("Type 'help' for available commands.\n");
      }
    }
  }

  /* Should never reach here */
  for (;;)
  {
    __asm__ volatile("hlt");
  }
}
