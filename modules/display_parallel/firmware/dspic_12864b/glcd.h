#ifndef GLCD_H
#define GLCD_H

// AliExpress '5V 12864 LCD Display Module 128x64 Dots Graphic Matrix LCD Blue Backlight'
// PCB 93x70 mm, label: 12864B
// ST7920 Controller

// All minimum timing requirements multiplied by 2 (fosc 270kHz instead of 540kHz)

// Pin 01 VSS
// Pin 02 VCC
// Pin 03 V0
// Pin 04 RS (RS=H:data  RS=L:instruction)
// Pin 05 R/W (RW=H:read  RW=L:write)
// Pin 06 E
// Pin 07 DB0
// Pin ......
// Pin 14 DB7
// Pin 15 PSB (PSB=H:parallel mode  PSB=L:serial mode)
// Pin 16 NC
// Pin 17 /RST
// Pin 18 VOUT
// Pin 19 LED A (+)
// Pin 20 LED K (-)

#include "common.h"

#define LCD_X                   128
#define LCD_Y                   64

#define GL_RS_INSTR             0
#define GL_RS_DATA              1
#define GL_PSB_SERIAL           0
#define GL_PSB_PARALLEL         1

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
#define GL_PSB                  _LATA2 // GL_CS1 == GL_!WR
#define GL_NC                   _LATA3 // GL_CS2 == GL_!RD

#define LCD_CMD_FUNCTION_SET    0x20 // bit4=DataLen, bit3=NrLines, bit2=Font
#define         FUNCTION_SET_8BIT      0x10 //(DL=1)
#define         FUNCTION_SET_4BIT      0x00 //(DL=0)
#define         FUNCTION_SET_EXTENDED  0x04 //(RE=1)
#define         FUNCTION_SET_BASIC     0x00 //(RE=0)
#define         FUNCTION_SET_G_ON      0x02 //(G=1)
#define         FUNCTION_SET_G_OFF     0x00 //(G=0)
#define LCD_EXT_RAM_ADDR        0x80 // BYTE0=bit5-0:vertical addr, BYTE1:bit3-0:horizontal 

#define lcd_ctrl_out()  (TRISA &= 0b1111111111101011)
#define lcd_dir_out()   (TRISB &= 0b0000000111101110)
#define lcd_out(x)      (LATB = (LATB&0b111101111) | ((x&0xFE)<<8) | ((x&0x01)<<4))


void lcd_write_command(u8 cmd);
void lcd_write_data(u8 data);
void lcd_bitmap(u8 *buff);
void lcd_init(void);

#endif
