#include "common.h"

s16 mpow(s16 x, s16 exp)
{
    s16 i;
    s16 number = 1;
    for (i=0; i<exp; ++i)
        number *= x;
    return(number);
}

void strcopy(char *dest, char *src)
{
    while (*src != 0) // copy without termination
        *dest++ = *src++;
}

char *mprint_int(char *s, s16 n, s16 base, s16 digits)
{
    s16 i;
    s16 sign=0;
    u16 num = n;

    if (base==10)
        if ((sign = n) < 0) // save sign
            num = -n; // make positive
    i = 0;
    do { // generate digits in reversed order
        s16 rem = num % base;
        s[i++] = (rem > 9)? (rem-10) + 'A' : rem + '0';
        digits--;
    } while (((num /= base) > 0)||(digits > 0));     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    i--;

    s16 j;
    for (j=0; j<=i/2; j++)
    {
        char c = s[i-j];
        s[i-j] = s[j];
        s[j] = c;
    }
    i++;
    s[i] = '\0';
    return &s[i]; // point to terminating char
}

char *mprint_float(char *str, float val, s16 pre_digits, s16 post_digits)
{
    s16 val_i = val;
    if (post_digits == 0)
        val_i = val + 0.5; // round up
    str = mprint_int(str, val_i, 10, pre_digits);
    if (post_digits > 0)
    {
        *str = '.';
        str++;
        s16 val_p = (val - (float)val_i) * (float)mpow(10,post_digits) + 0.5;
        str = mprint_int(str, val_p, 10, post_digits);
    }
    return str;
}