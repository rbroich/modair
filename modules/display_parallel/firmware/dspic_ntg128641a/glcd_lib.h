#ifndef GLCD_LIB_H
#define GLCD_LIB_H

#include "common.h"

#define GLCD_COLOR_WHITE   0
#define GLCD_COLOR_BLACK   1
#define GLCD_FONT_3x5      0
#define GLCD_FONT_4x5      1
#define GLCD_FONT_5x7      2
#define GLCD_ROTATE_0      0
#define GLCD_ROTATE_90     1
#define GLCD_ROTATE_180    2
#define GLCD_ROTATE_270    3

void LCD_string(u8 *buff, char *str, u8 x, u8 y, u8 rotate, u8 font, u8 color, u8 max_char);
void LCD_char(u8 *buff, char digit, u8 x, u8 y, u8 rotate, u8 font, u8 color);
void LCD_rect(u8 *buff, u8 x1, u8 y1, u8 x2, u8 y2, u8 color, u8 fill);
void LCD_line(u8 *buff, u8 x1, u8 y1, u8 x2, u8 y2, u8 color);
void LCD_circle(u8 *buff, u8 xpos, u8 ypos, u8 rad, u8 color);
void LCD_setpixel(u8 *buff, u8 x, u8 y, u8 color);
void LCD_dot(u8 *buff, u8 x, u8 y, u8 thick, u8 color);

#endif
