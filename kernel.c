/* kernel.c - Main kernel with process manager and scheduler */
#include "memory.h"
#include "serial.h"
#include "string.h"
#include "types.h"
#include "process.h"
#include "scheduler.h"
#include <string.h>
#define MAX_INPUT 128

/* Example process 1: Counter process */
void process_counter(void)
{
  int count = 0;
  while (1)
  {
    serial_puts("[Counter] Count: ");
    /* Simple counter display */
    count++;

    /* Yield to other processes */
    scheduler_yield();

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
  while (1)
  {
    /* Just yield CPU */
    scheduler_yield();
    __asm__ volatile("hlt");
  }
}

/* Example process 3: Worker process */
void process_worker(void)
{
  int work_units = 0;
  while (1)
  {
    work_units++;

    /* Simulate work */
    int i;
    for (i = 0; i < 500000; i++)
    {
      __asm__ volatile("nop");
    }

    scheduler_yield();
  }
}

void kmain(void)
{
  char input[MAX_INPUT];
  int pos = 0;

  /* Initialize hardware */
  serial_init();
  memory_init();
  process_init();
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
