#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

/* Fixed block size for simple allocation */
#define BLOCK_SIZE 1048576 /* 1MB blocks */
#define MAX_BLOCKS 32      /* Maximum number of blocks (32MB for 32MB system) */

/* Memory block header for free list */
typedef struct mem_block
{
    uint32_t is_free;       /* 1 if free, 0 if allocated */
    struct mem_block *next; /* Next block in the list */
} mem_block_t;

/* Initialize memory manager */
void memory_init(void);

/* Heap allocation - allocate block of memory of given size */
void *kmalloc(size_t size);

/* Heap deallocation - free previously allocated memory */
void kfree(void *ptr);

/* Stack allocation - allocate stack for process */
void *stack_alloc(size_t size);

/* Stack deallocation - free process stack */
void stack_free(void *stack_base, size_t size);

/* Print memory statistics */
void memory_dump_stats(void);

#endif // !MEMORY_H
