/* helper.c - Helper utility functions implementation */
#include "helper.h"

/* Convert integer to string */
void int_to_str(int num, char *str)
{
    int i = 0;
    int is_negative = 0;

    if (num == 0)
    {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    if (num < 0)
    {
        is_negative = 1;
        num = -num;
    }

    while (num > 0)
    {
        str[i++] = (num % 10) + '0';
        num /= 10;
    }

    if (is_negative)
    {
        str[i++] = '-';
    }

    str[i] = '\0';

    /* Reverse the string */
    int j;
    for (j = 0; j < i / 2; j++)
    {
        char temp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = temp;
    }
}

/* Set memory to a specific value */
void memset(void *dest, uint8_t val, uint32_t count)
{
    uint8_t *temp = (uint8_t *)dest;
    uint32_t i;
    for (i = 0; i < count; i++)
    {
        temp[i] = val;
    }
}

/* Convert integer to hex string */
void int_to_hex(uint32_t num, char *str)
{
    const char hex_digits[] = "0123456789ABCDEF";
    int i;
    
    for (i = 7; i >= 0; i--)
    {
        str[i] = hex_digits[num & 0xF];
        num >>= 4;
    }
    str[8] = '\0';
}
