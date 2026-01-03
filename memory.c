#include "memory.h"
#include "serial.h"
#include "helper.h"

/* __kernel_end defined in link.ld */
extern uint32_t __kernel_end;

/* Global pointer to block list head */
static mem_block_t *block_head = NULL;

/* Memory pool */
static uint32_t memory_pool_start = 0;
static uint32_t total_blocks = 0;

/* Memory statistics */
static uint32_t total_allocated = 0;
static uint32_t total_freed = 0;
static uint32_t num_allocations = 0;
static uint32_t num_frees = 0;

/* Helper: Align address to 4-byte boundary */
static uint32_t align_4(uint32_t addr)
{
  return (addr + 3) & ~0x3;
}

/* Initialize memory manager */
void memory_init(void)
{
  uint32_t i;
  mem_block_t *current_block;

  /* Start memory pool after kernel */
  memory_pool_start = align_4((uint32_t)&__kernel_end);

  /* Calculate how many blocks we can create */
  total_blocks = MAX_BLOCKS;

  /* Initialize block head */
  block_head = (mem_block_t *)memory_pool_start;

  /* Create linked list of fixed-size blocks */
  current_block = block_head;
  for (i = 0; i < total_blocks; i++)
  {
    current_block->is_free = 1;

    /* Calculate next block address */
    if (i < total_blocks - 1)
    {
      current_block->next = (mem_block_t *)((uint32_t)current_block + sizeof(mem_block_t) + BLOCK_SIZE);
      current_block = current_block->next;
    }
    else
    {
      current_block->next = NULL; /* Last block */
    }
  }

  total_allocated = 0;
  total_freed = 0;
  num_allocations = 0;
  num_frees = 0;

  serial_puts("Memory manager initialized (Fixed-size blocks).\n");
  serial_puts("Block size: ");
  char num_str[12];
  int_to_str(BLOCK_SIZE, num_str);
  serial_puts(num_str);
  serial_puts(" bytes\n");
  serial_puts("Total blocks: ");
  int_to_str(total_blocks, num_str);
  serial_puts(num_str);
  serial_puts("\n");
  serial_puts("Memory pool starts at: 0x");
  int_to_str(memory_pool_start, num_str);
  serial_puts(num_str);
  serial_puts("\n");
}

/* Heap allocation using first-fit strategy */
void *kmalloc(size_t size)
{
  mem_block_t *current;

  if (size == 0 || size > BLOCK_SIZE)
  {
    serial_puts("ERROR: kmalloc() - invalid size\n");
    return NULL;
  }

  /* First-fit: Find first free block */
  current = block_head;
  while (current != NULL)
  {
    if (current->is_free)
    {
      /* Found a free block */
      current->is_free = 0;

      /* Update statistics */
      num_allocations++;
      total_allocated += BLOCK_SIZE;

      /* Return pointer after header */
      return (void *)((uint32_t)current + sizeof(mem_block_t));
    }
    current = current->next;
  }

  /* No free block found */
  serial_puts("ERROR: kmalloc() - out of memory\n");
  return NULL;
}

/* Heap deallocation */
void kfree(void *ptr)
{
  mem_block_t *block;

  if (ptr == NULL)
  {
    return;
  }

  /* Get block header */
  block = (mem_block_t *)((uint32_t)ptr - sizeof(mem_block_t));

  /* Validate block is in our memory pool */
  if ((uint32_t)block < memory_pool_start)
  {
    serial_puts("ERROR: kfree() - invalid pointer\n");
    return;
  }

  /* Check if already free */
  if (block->is_free)
  {
    serial_puts("WARNING: kfree() - double free detected\n");
    return;
  }

  /* Mark as free */
  block->is_free = 1;

  /* Update statistics */
  num_frees++;
  total_freed += BLOCK_SIZE;
}

/* Stack allocation - wrapper around kmalloc for clarity */
void *stack_alloc(size_t size)
{
  return kmalloc(size);
}

/* Stack deallocation - wrapper around kfree for clarity */
void stack_free(void *stack_base, size_t size)
{
  (void)size; /* Unused parameter */
  kfree(stack_base);
}

/* Print memory statistics */
void memory_dump_stats(void)
{
  char num_str[12];
  uint32_t free_blocks = 0;
  uint32_t used_blocks = 0;
  mem_block_t *current;

  /* Count free and used blocks */
  current = block_head;
  while (current != NULL)
  {
    if (current->is_free)
    {
      free_blocks++;
    }
    else
    {
      used_blocks++;
    }
    current = current->next;
  }

  serial_puts("\n=== Memory Statistics ===\n");

  serial_puts("Block size:       ");
  int_to_str(BLOCK_SIZE, num_str);
  serial_puts(num_str);
  serial_puts(" bytes\n");

  serial_puts("Total blocks:     ");
  int_to_str(total_blocks, num_str);
  serial_puts(num_str);
  serial_puts("\n");

  serial_puts("Free blocks:      ");
  int_to_str(free_blocks, num_str);
  serial_puts(num_str);
  serial_puts("\n");

  serial_puts("Used blocks:      ");
  int_to_str(used_blocks, num_str);
  serial_puts(num_str);
  serial_puts("\n");

  serial_puts("Free memory:      ");
  int_to_str(free_blocks * BLOCK_SIZE, num_str);
  serial_puts(num_str);
  serial_puts(" bytes\n");

  serial_puts("Used memory:      ");
  int_to_str(used_blocks * BLOCK_SIZE, num_str);
  serial_puts(num_str);
  serial_puts(" bytes\n");

  serial_puts("Allocations:      ");
  int_to_str(num_allocations, num_str);
  serial_puts(num_str);
  serial_puts("\n");

  serial_puts("Frees:            ");
  int_to_str(num_frees, num_str);
  serial_puts(num_str);
  serial_puts("\n\n");
}
