#include "common.h"


s16 linear_interpolate(u16 x, u16 x0, u16 x1, s16 y0, s16 y1)
{
    s32 sub_xx0 = x-x0;
    s32 subx1x0 = x1-x0;
    s32 suby1y0 = y1-y0;
    s32 mul0 = suby1y0*sub_xx0;
    s32 div0 = mul0/subx1x0;
    return (y0 + div0);
}

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
    // DO NOT TERMINATE!!!
    i++;
    return &s[i]; // point to char after last
}

char *mprint_float(char *str, float val, s16 pre_digits, s16 post_digits)
{
    s16 val_i = val;
	s16 val_p;
	s16 ten_pow_post;
    if (post_digits == 0)
        val_i = val + 0.5; // round up
    else {
		ten_pow_post = mpow(10,post_digits);
        val_p = (val - (float)val_i) * (float)ten_pow_post + 0.5;
		if (val_p >= ten_pow_post) { // caused rounding up (e.g. 10.997 with post_digits==2 should print as 11.00)
			val_i++;
			val_p = 0;
		}
	}
    str = mprint_int(str, val_i, 10, pre_digits);
    if (post_digits > 0) {
        *str = '.';
        str++;
        str = mprint_int(str, val_p, 10, post_digits);
    }
    return str;
}