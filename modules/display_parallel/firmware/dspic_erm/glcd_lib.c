#include "glcd_lib.h"
#include <string.h>

#include "glcd.h"
#include "glcd_font3x5.h"
#include "glcd_font4x5.h"
#include "glcd_font5x7.h"

u8 lcd_buff[LCD_X*LCD_Y/8];

void LCD_fill_buffer(u8 color)
{
    if (color == GLCD_COLOR_BLACK)
        memset(lcd_buff,0xFF,sizeof(lcd_buff));
    if (color == GLCD_COLOR_WHITE)
        memset(lcd_buff,0x00,sizeof(lcd_buff));
}

void LCD_write_buffer()
{
    lcd_init();
    lcd_bitmap(lcd_buff);
}

void LCD_string(char *str, u8 x, u8 y, u8 rotate, u8 font, u8 color, u8 max_char)
{
    u8 fwidth = 0;
    switch (font)
    {
        case GLCD_FONT_3x5: fwidth = 4; break;
        case GLCD_FONT_4x5: fwidth = 4; break;
        case GLCD_FONT_5x7: fwidth = 6; break;
    }
    while ((*str)&&(max_char))
    {
        LCD_char(*str,x,y,rotate,font,color);
        str++;
        max_char--;
        switch (rotate)
        {
            case GLCD_ROTATE_0: x+=fwidth; break;
            case GLCD_ROTATE_90: y+=fwidth; break;
            case GLCD_ROTATE_180: x-=fwidth; break;
            case GLCD_ROTATE_270: y-=fwidth; break;
        }
    }
}

void LCD_char(char digit, u8 x, u8 y, u8 rotate, u8 font, u8 color)
{
    u8 *ptr;
    u8 i,j;
    if (font == GLCD_FONT_3x5)
    {
        ptr = (u8 *)&font3x5[(digit-' ')*3];
        for (j=0;j<3;j++)
            for (i=0;i<5;i++)
                if (ptr[j] & (1<<i))
                    switch (rotate)
                    {
                        case GLCD_ROTATE_0: LCD_setpixel(x+j-1, y+i-2, color); break; // 0 degrees
                        case GLCD_ROTATE_90: LCD_setpixel(x-i+2, y+j-1, color); break; // 90 degrees
                        case GLCD_ROTATE_180: LCD_setpixel(x-j+1, y-i+2, color); break; // 180 degrees
                        case GLCD_ROTATE_270: LCD_setpixel(x+i-2, y-j+1, color); break; // 270 degrees
                    }
    }
    if (font == GLCD_FONT_4x5)
    {
        // actually also 3x5
        ptr = (u8 *)&font4x5[(digit-' ')*3];
        for (j=0;j<3;j++)
            for (i=0;i<5;i++)
                if (ptr[j] & (1<<i))
                    switch (rotate)
                    {
                        case GLCD_ROTATE_0: LCD_setpixel(x+j-1, y+i-2, color); break; // 0 degrees
                        case GLCD_ROTATE_90: LCD_setpixel(x-i+2, y+j-1, color); break; // 90 degrees
                        case GLCD_ROTATE_180: LCD_setpixel(x-j+1, y-i+2, color); break; // 180 degrees
                        case GLCD_ROTATE_270: LCD_setpixel(x+i-2, y-j+1, color); break; // 270 degrees
                    }
    }
    if (font == GLCD_FONT_5x7)
    {
        ptr = (u8 *)&font5x7[(digit-' ')*5];
        for (j=0;j<5;j++)
            for (i=0;i<7;i++)
                if (ptr[j] & (1<<i))
                    switch (rotate)
                    {
                        case GLCD_ROTATE_0: LCD_setpixel(x+j-2, y+i-3, color); break; // 0 degrees
                        case GLCD_ROTATE_90: LCD_setpixel(x-i+3, y+j-2, color); break; // 90 degrees
                        case GLCD_ROTATE_180: LCD_setpixel(x-j+2, y-i+3, color); break; // 180 degrees
                        case GLCD_ROTATE_270: LCD_setpixel(x+i-3, y-j+2, color); break; // 270 degrees
                    }
    }
}

void LCD_rect(u8 x1, u8 y1, u8 x2, u8 y2, u8 color, u8 fill)
{
    if (fill)
    {
        u16 i;
        u16 j;
        for (i=x1;i<=x2;i++)
            for (j=y1;j<=y2;j++)
                LCD_setpixel(i, j, color);
    } else {
        u16 i;
        for (i=x1;i<=x2;i++)
            LCD_setpixel(i, y1, color);
        for (i=x1;i<=x2;i++)
            LCD_setpixel(i, y2, color);
        for (i=y1;i<=y2;i++)
            LCD_setpixel(x1, i, color);
        for (i=y1;i<=y2;i++)
            LCD_setpixel(x2, i, color);
    }
}

void LCD_line(u8 x1, u8 y1, u8 x2, u8 y2, u8 color)
{
    s8 x, y, xinc1, xinc2, yinc1, yinc2, den, num, numadd, numpixels, curpixel, deltax, deltay;

    if (x2 < x1) // The difference between the x's
        deltax = x1 - x2;
    else deltax = x2 - x1; // abs(x2 - x1)
    if (y2 < y1) // The difference between the y's
        deltay = y1 - y2;
    else deltay = y2 - y1; // abs(y2 - y1)
    x = x1; // Start x off at the first pixel
    y = y1; // Start y off at the first pixel

    if (x2 >= x1) // The x-values are increasing
    {
        xinc1 = 1;
        xinc2 = 1;
    } else { // The x-values are decreasing
        xinc1 = -1;
        xinc2 = -1;
    }

    if (y2 >= y1) // The y-values are increasing
    {
        yinc1 = 1;
        yinc2 = 1;
    } else { // The y-values are decreasing
        yinc1 = -1;
        yinc2 = -1;
    }

    if (deltax >= deltay) // There is at least one x-value for every y-value
    {
        xinc1 = 0; // Don't change the x when numerator >= denominator
        yinc2 = 0; // Don't change the y for every iteration
        den = deltax;
        num = deltax / 2;
        numadd = deltay;
        numpixels = deltax; // There are more x-values than y-values
    } else { // There is at least one y-value for every x-value
        xinc2 = 0; // Don't change the x for every iteration
        yinc1 = 0; // Don't change the y when numerator >= denominator
        den = deltay;
        num = deltay / 2;
        numadd = deltax;
        numpixels = deltay; // There are more y-values than x-values
    }

    for (curpixel = 0; curpixel <= numpixels; curpixel++)
    {
        LCD_setpixel(x, y, color); // Draw the current pixel
        num += numadd; // Increase the numerator by the top of the fraction
        if (num >= den) // Check if numerator >= denominator
        {
            num -= den; // Calculate the new numerator value
            x += xinc1; // Change the x as appropriate
            y += yinc1; // Change the y as appropriate
        }
        x += xinc2; // Change the x as appropriate
        y += yinc2; // Change the y as appropriate
    }
}

void LCD_circle(u8 xpos, u8 ypos, u8 rad, u8 color)
{
    u8 y,x;
    s8 p;
    x = 0;
    y = rad;
    p = -rad/2;
    LCD_setpixel(xpos + x, ypos + y, color);
    LCD_setpixel(xpos + x, ypos - y, color);
    LCD_setpixel(xpos - x, ypos + y, color);
    LCD_setpixel(xpos - x, ypos - y, color);
    LCD_setpixel(xpos + y, ypos + x, color);
    LCD_setpixel(xpos + y, ypos - x, color);
    LCD_setpixel(xpos - y, ypos + x, color);
    LCD_setpixel(xpos - y, ypos - x, color);
    while(x<y)
    {
        x++;
        if(p<0)
            p=p+2*x+1;
        else
        {
            y--;
            p=p+2*(x-y)+1;
        }
        LCD_setpixel(xpos + x, ypos + y, color);
        LCD_setpixel(xpos + x, ypos - y, color);
        LCD_setpixel(xpos - x, ypos + y, color);
        LCD_setpixel(xpos - x, ypos - y, color);
        LCD_setpixel(xpos + y, ypos + x, color);
        LCD_setpixel(xpos + y, ypos - x, color);
        LCD_setpixel(xpos - y, ypos + x, color);
        LCD_setpixel(xpos - y, ypos - x, color);
    }
}

void LCD_dot(u8 x, u8 y, u8 thick, u8 color)
{
    LCD_setpixel(x+0,y+0,color);
    if (thick == 0) return;
    LCD_setpixel(x+1,y+0,color);
    LCD_setpixel(x+0,y+1,color);
    LCD_setpixel(x-1,y+0,color);
    LCD_setpixel(x+0,y-1,color);
    if (thick == 1) return;
    LCD_setpixel(x-1,y+1,color);
    LCD_setpixel(x+1,y+1,color);
    LCD_setpixel(x+2,y+0,color);
    LCD_setpixel(x+2,y+1,color);
    LCD_setpixel(x+1,y-1,color);
    LCD_setpixel(x+0,y+2,color);
    LCD_setpixel(x+1,y+2,color);
    if (thick == 2) return;
    LCD_setpixel(x-1,y-1,color);
    LCD_setpixel(x-1,y+2,color);
    LCD_setpixel(x+2,y-1,color);
    LCD_setpixel(x-2,y-1,color);
    LCD_setpixel(x-2,y+0,color);
    LCD_setpixel(x-2,y+1,color);
    LCD_setpixel(x-1,y-2,color);
    LCD_setpixel(x+0,y-2,color);
    LCD_setpixel(x+1,y-2,color);
}

void LCD_setpixel(u8 x, u8 y, u8 color)
{
    if ((x>=LCD_X)||(y>=LCD_Y))
        return;
    if (color == GLCD_COLOR_BLACK)
        lcd_buff[y*LCD_X/8 + ((x & 0xF8) >> 3)] |= (1<<(7-(x&0x07))); // set bit
    if (color == GLCD_COLOR_WHITE)
        lcd_buff[y*LCD_X/8 + ((x & 0xF8) >> 3)] &= (~(1<<(7-(x&0x07)))); // clear bit
}

