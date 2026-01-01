#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

/* initialize mem mangaer */
void memory_init(void);

/* Allocate block of memory of size */
void *kmalloc(size_t size);

/* Print current heap pointer */
void memory_dump_stats(void);

#endif // !MEMORY_H
