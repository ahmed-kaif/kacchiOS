/* kernel.c - Main kernel with process manager and scheduler */
#include "memory.h"
#include "serial.h"
#include "string.h"
#include "types.h"
#include "process.h"
#include "scheduler.h"
#include "idt.h"
#include "pic.h"
#include "timer.h"
#include "helper.h"
#include <string.h>
#define MAX_INPUT 128

/* Example process 1: Counter process */
void process_counter(void)
{
  int count = 0;
  serial_puts("[Counter] Started\n");

  while (1)
  {
    if (count % 5 == 0)
    {
      serial_puts("[Counter] Count: ");
      char num_str[12];
      int_to_str(count, num_str);
      serial_puts(num_str);
      serial_puts("\n");
    }

    count++;

    /* Simulate some work */
    int i;
    for (i = 0; i < 1000000; i++)
    {
      __asm__ volatile("nop");
    }
  }
}

/* Example process 2: Idle process */
void process_idle(void)
{
  serial_puts("[Idle] Started\n");
  while (1)
  {
    /* Just halt and wait for timer interrupt */
    __asm__ volatile("hlt");
  }
}

/* Example process 3: Worker process */
void process_worker(void)
{
  int work_units = 0;
  serial_puts("[Worker] Started\n");

  while (1)
  {
    work_units++;

    if (work_units % 10 == 0)
    {
      serial_puts("[Worker] Work units: ");
      char num_str[12];
      int_to_str(work_units, num_str);
      serial_puts(num_str);
      serial_puts("\n");
    }

    /* Simulate work */
    int i;
    for (i = 0; i < 500000; i++)
    {
      __asm__ volatile("nop");
    }
  }
}

void kmain(void)
{
  char input[MAX_INPUT];
  int pos = 0;

  /* Initialize hardware */
  serial_init();

  /* Initialize interrupt system */
  idt_init();
  pic_init();

  /* Initialize subsystems */
  memory_init();
  process_init();
  scheduler_init();

  /* Initialize timer (100 Hz = 10ms per tick) */
  timer_init(100);

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
  serial_puts("  ticks    - Show timer ticks\n");
  serial_puts("  help     - Show this help message\n");
  serial_puts("  exit     - Halt the system\n");
  serial_puts("\n");

  /* Create example processes (currently for demonstration) */
  serial_puts("Creating demo processes...\n");
  process_create("idle", process_idle, PRIORITY_LOW);
  process_create("worker1", process_worker, PRIORITY_NORMAL);
  process_create("counter", process_counter, PRIORITY_NORMAL);
  serial_puts("\n");

  /* Main loop - the "null process" */
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
        serial_puts("Note: Starting scheduler will begin process execution.\n");
        serial_puts("The system will run processes continuously.\n");
        scheduler_start();
        /* Never returns - scheduler takes over */
      }
      else if (strcmp(input, "stats") == 0)
      {
        scheduler_stats();
      }
      else if (strcmp(input, "mem") == 0)
      {
        memory_dump_stats();
      }
      else if (strcmp(input, "ticks") == 0)
      {
        serial_puts("Timer ticks: ");
        char ticks_str[12];
        int_to_str(timer_get_ticks(), ticks_str);
        serial_puts(ticks_str);
        serial_puts("\n");
      }
      else if (strcmp(input, "help") == 0)
      {
        serial_puts("Available commands:\n");
        serial_puts("  ps       - List all processes\n");
        serial_puts("  start    - Start the scheduler\n");
        serial_puts("  stats    - Show scheduler statistics\n");
        serial_puts("  mem      - Show memory statistics\n");
        serial_puts("  ticks    - Show timer ticks\n");
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
