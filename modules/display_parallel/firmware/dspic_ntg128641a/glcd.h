#ifndef GLCD_H
#define GLCD_H

// 2.9" NelyTech NT-G128641A 128x64 Graphic LCD Display Module, Black on White
// PCB 93x70 mm
// 2 x KS0108 Controllers

// All minimum timing requirements multiplied by 2 (fosc 270kHz instead of 540kHz)

// Pin 01 VSS
// Pin 02 VDD
// Pin 03 V0
// Pin 04 RS (RS=H:data  RS=L:instruction)
// Pin 05 R/W (RW=H:read  RW=L:write)
// Pin 06 E
// Pin 07 DB0
// Pin ......
// Pin 14 DB7
// Pin 15 CS1
// Pin 16 CS2
// Pin 17 /RST
// Pin 18 VOUT
// Pin 19 LED A (+)
// Pin 20 LED K (-)

#include "common.h"

#define LCD_X                   64
#define LCD_Y                   128
#define LCD_WHITE               0
#define LCD_BLACK               1

#define GL_RS_INSTR             0
#define GL_RS_DATA              1

#define GL_RS                   _LATA4 // GL_CD
#define GL_E                    _LATB0
#define GL_D0                   _LATB4
#define GL_D1                   _LATB9
#define GL_D2                   _LATB10
#define GL_D3                   _LATB11
#define GL_D4                   _LATB12
#define GL_D5                   _LATB13
#define GL_D6                   _LATB14
#define GL_D7                   _LATB15
#define GL_CS1                  _LATA2 // GL_CS1 == GL_!WR
#define GL_CS2                  _LATA3 // GL_CS2 == GL_!RD

#define LCD_SET_ADDR            0x40
#define LCD_SET_PAGE            0xB8
#define LCD_START_LINE          0xC0
#define LCD_OFF                 0x3E
#define LCD_ON                  0x3F

#define lcd_ctrl_out()  (TRISA &= 0b1111111111100011)
#define lcd_dir_out()   (TRISB &= 0b0000000111101110)
#define lcd_data_out(x) (LATB = (LATB&0b111101111) | ((x&0xFE)<<8) | ((x&0x01)<<4))



void lcd_out(u8 val);
void lcd_cmd(u8 val);
void lcd_init(void);
void lcd_fill(u8 color);
void lcd_update(void);
void lcd_clrbuff(void);
void lcd_setpixel(u8 x, u8 y, u8 color);

#endif
