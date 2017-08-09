#include "glcd_lib.h"

#include "glcd_font3x5.h"
#include "glcd_font4x5.h"
#include "glcd_font5x7.h"

void LCD_string(char *str, u8 x, u8 y, u8 rotate, u8 font, u8 color, u8 max_char)
{
    u8 fwidth = 0;
    switch (font) {
        case GLCD_FONT_3x5: fwidth = 4; break;
        case GLCD_FONT_4x5: fwidth = 4; break;
        case GLCD_FONT_5x7: fwidth = 6; break;
    }
    while ((*str)&&(max_char)) {
        LCD_char(*str,x,y,rotate,font,color);
        str++;
        max_char--;
        switch (rotate) { // rotation: clock-wise
            case GLCD_ROTATE_0: x+=fwidth; break; // left to right, (x,y) are top left corner
            case GLCD_ROTATE_90: y-=fwidth; break; // bottom to top, (x,y) are bottom left corner
            case GLCD_ROTATE_180: x-=fwidth; break; // right to left, (x,y) are bottom right corner
            case GLCD_ROTATE_270: y+=fwidth; break; // top to bottom, (x,y) are top right corner
        }
    }
}

void LCD_char(char digit, u8 x, u8 y, u8 rotate, u8 font, u8 color)
{
    u8 *ptr;
    u8 fy,fx; // 0,0 = top left in font coordinate system
    u8 fymax,fxmax;
    switch (font) {
        case GLCD_FONT_3x5: fxmax=3;fymax=5;ptr=(u8 *)&font3x5[(digit-' ')*3]; break;
        case GLCD_FONT_4x5: fxmax=3;fymax=5;ptr=(u8 *)&font4x5[(digit-' ')*3]; break;
        case GLCD_FONT_5x7: fxmax=5;fymax=7;ptr=(u8 *)&font5x7[(digit-' ')*5]; break;
    }
    for (fx=0;fx<fxmax;fx++) // loop over font coordinates
    for (fy=0;fy<fymax;fy++)
        if (ptr[fx] & (1<<fy)) // set appropriate pixel if the font pixel is active
        switch (rotate) {
            case GLCD_ROTATE_0: lcd_setpixel(x+fx, y+fy, color); break; // 0 degrees
            case GLCD_ROTATE_90: lcd_setpixel(x+fy, y-fx, color); break; // 90 degrees
            case GLCD_ROTATE_180: lcd_setpixel(x-fx, y-fy, color); break; // 180 degrees
            case GLCD_ROTATE_270: lcd_setpixel(x-fy, y+fx, color); break; // 270 degrees
        }
}

void LCD_rect(u8 x1, u8 y1, u8 x2, u8 y2, u8 color, u8 fill)
{
    if (fill) {
        u16 i;
        u16 j;
        for (i=x1;i<=x2;i++)
            for (j=y1;j<=y2;j++)
                lcd_setpixel(i, j, color);
    } else {
        u16 i;
        for (i=x1;i<=x2;i++)
            lcd_setpixel(i, y1, color);
        for (i=x1;i<=x2;i++)
            lcd_setpixel(i, y2, color);
        for (i=y1;i<=y2;i++)
            lcd_setpixel(x1, i, color);
        for (i=y1;i<=y2;i++)
            lcd_setpixel(x2, i, color);
    }
}

void LCD_line(u8 x1, u8 y1, u8 x2, u8 y2, u8 color)
{
    s16 x, y, xinc1, xinc2, yinc1, yinc2, den, num, numadd, numpixels, curpixel, deltax, deltay;

    if (x2 < x1) // The difference between the x's
        deltax = x1 - x2;
    else deltax = x2 - x1; // abs(x2 - x1)
    if (y2 < y1) // The difference between the y's
        deltay = y1 - y2;
    else deltay = y2 - y1; // abs(y2 - y1)
    x = x1; // Start x off at the first pixel
    y = y1; // Start y off at the first pixel

    if (x2 >= x1) { // The x-values are increasing
        xinc1 = 1;
        xinc2 = 1;
    } else { // The x-values are decreasing
        xinc1 = -1;
        xinc2 = -1;
    }

    if (y2 >= y1) { // The y-values are increasing
        yinc1 = 1;
        yinc2 = 1;
    } else { // The y-values are decreasing
        yinc1 = -1;
        yinc2 = -1;
    }

    if (deltax >= deltay) { // There is at least one x-value for every y-value
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
        lcd_setpixel(x, y, color); // Draw the current pixel
        num += numadd; // Increase the numerator by the top of the fraction
        if (num >= den) { // Check if numerator >= denominator
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
    lcd_setpixel(xpos + x, ypos + y, color);
    lcd_setpixel(xpos + x, ypos - y, color);
    lcd_setpixel(xpos - x, ypos + y, color);
    lcd_setpixel(xpos - x, ypos - y, color);
    lcd_setpixel(xpos + y, ypos + x, color);
    lcd_setpixel(xpos + y, ypos - x, color);
    lcd_setpixel(xpos - y, ypos + x, color);
    lcd_setpixel(xpos - y, ypos - x, color);
    while(x<y) {
        x++;
        if(p<0)
            p=p+2*x+1;
        else {
            y--;
            p=p+2*(x-y)+1;
        }
        lcd_setpixel(xpos + x, ypos + y, color);
        lcd_setpixel(xpos + x, ypos - y, color);
        lcd_setpixel(xpos - x, ypos + y, color);
        lcd_setpixel(xpos - x, ypos - y, color);
        lcd_setpixel(xpos + y, ypos + x, color);
        lcd_setpixel(xpos + y, ypos - x, color);
        lcd_setpixel(xpos - y, ypos + x, color);
        lcd_setpixel(xpos - y, ypos - x, color);
    }
}

void LCD_dot(u8 x, u8 y, u8 thick, u8 color)
{
    lcd_setpixel(x+0,y+0,color);
    if (thick == 0) return;
    lcd_setpixel(x+1,y+0,color);
    lcd_setpixel(x+0,y+1,color);
    lcd_setpixel(x-1,y+0,color);
    lcd_setpixel(x+0,y-1,color);
    if (thick == 1) return;
    lcd_setpixel(x-1,y+1,color);
    lcd_setpixel(x+1,y+1,color);
    lcd_setpixel(x+2,y+0,color);
    lcd_setpixel(x+2,y+1,color);
    lcd_setpixel(x+1,y-1,color);
    lcd_setpixel(x+0,y+2,color);
    lcd_setpixel(x+1,y+2,color);
    if (thick == 2) return;
    lcd_setpixel(x-1,y-1,color);
    lcd_setpixel(x-1,y+2,color);
    lcd_setpixel(x+2,y-1,color);
    lcd_setpixel(x-2,y-1,color);
    lcd_setpixel(x-2,y+0,color);
    lcd_setpixel(x-2,y+1,color);
    lcd_setpixel(x-1,y-2,color);
    lcd_setpixel(x+0,y-2,color);
    lcd_setpixel(x+1,y-2,color);
}

