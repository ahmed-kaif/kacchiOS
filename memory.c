#include "memory.h"
#include "serial.h"

/* __kernel_end defined in link.ld */
extern uint32_t __kernel_end;

/* pointer to next free byte of mem */
static uint32_t free_mem_addr = 0;

void memory_init(void) {
  free_mem_addr = (uint32_t)&__kernel_end;
  serial_puts("Memory manager initialized.\n");
  serial_puts("Heap starts at: 0x");
  // TODO: print_hex for heap addr
  serial_puts("[Kernel End Addr]");
}

void *kmalloc(size_t size) {
  /* 4 byte allignment of memory */
  if (free_mem_addr & 0xFFFFF003) {
    free_mem_addr &= 0xFFFFFFFC;
    free_mem_addr += 4;
  }
  uint32_t ret = free_mem_addr;

  free_mem_addr = free_mem_addr + size;

  return (void *)ret;
}

void memory_dump_stats(void) {
  serial_puts("Current Heap Pointer: 0x");
  /* TODO: Implement hex printing to see the actual address */
  serial_puts("...\n");
}
