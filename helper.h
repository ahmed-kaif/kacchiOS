/* helper.h - Helper utility functions */
#ifndef HELPER_H
#define HELPER_H

#include "types.h"

/* Convert integer to string */
void int_to_str(int num, char *str);

/* Set memory to a specific value */
void memset(void *dest, uint8_t val, uint32_t count);

#endif /* HELPER_H */
