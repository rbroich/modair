#ifndef GLCD_H
#define GLCD_H

#include "common.h"

#define LCD_Y           64
#define LCD_X           256
#define LCD_CHAR_WIDTH  8 // 6 or 8 bits depending on FS (H:6x8 L:8x8)

#define LCD_D7          _RB15
#define LCD_D6          _RB14
#define LCD_D5          _RB13
#define LCD_D4          _RB12
#define LCD_D3          _RB11
#define LCD_D2          _RB10
#define LCD_D1          _RB9
#define LCD_D0          _RB4
#define LCD_nWE         _LATA2
#define LCD_nRE         _LATA3
#define LCD_CD          _LATA4

#define lcd_out(x)      (LATB = (LATB&0b111101111) | ((x&0xFE)<<8) | ((x&0x01)<<4))
#define lcd_in(x)       (x = ((PORTB&0xFE00) >> 8) | ((PORTB&0x0010)>>4))
#define lcd_dir_in()    (TRISB |= 0b1111111000010000)
#define lcd_dir_out()   (TRISB &= 0b0000000111101111)
#define lcd_ctrl_out()  (TRISA &= 0b1111111111100011)
#define lcd_cd_cmd()    (LCD_CD=1)
#define lcd_cd_data()   (LCD_CD=0)
#define lcd_rw_idle()   {LCD_nWE=1;LCD_nRE=1;}
#define lcd_rw_rd()     {LCD_nWE=1;LCD_nRE=0;}
#define lcd_rw_wr()     {LCD_nWE=0;LCD_nRE=1;}

/* Define the Memory Map */
#define LCD_GRAPHICS_HOME   0x0000  /* This will usually be at the start of RAM */
#define LCD_GRAPHICS_AREA   0x14    /* A graphic character is 8 bits wide (same as 8x8 char) */
#define LCD_GRAPHICS_SIZE   0x0800  /* Size of graphics RAM */
#define LCD_TEXT_HOME       0x0A00  /* Graphics Area + Text Attribute Size (same size as text size) */
#define LCD_TEXT_AREA       0x14    /* Text line is 20 chars wide */
#define LCD_TEXT_SIZE       0x0200  /* Size of text RAM */

// Control Words
#define LCD_CURSOR_POINTER_SET          0b00100001
#define LCD_OFFSET_REGISTER_SET         0b00100010
#define LCD_ADDRESS_POINTER_SET         0b00100100
#define LCD_TEXT_HOME_ADDRESS_SET       0b01000000
#define LCD_TEXT_AREA_SET               0b01000001
#define LCD_GRAPHIC_HOME_ADDRESS_SET    0b01000010
#define LCD_GRAPHIC_AREA_SET            0b01000011

#define LCD_CG_ROM_MODE_OR              0b10000000
#define LCD_CG_ROM_MODE_EXOR            0b10000001
#define LCD_CG_ROM_MODE_AND             0b10000011
#define LCD_CG_ROM_MODE_TEXT            0b10000100
#define LCD_CG_RAM_MODE_OR              0b10001000
#define LCD_CG_RAM_MODE_EXOR            0b10001001
#define LCD_CG_RAM_MODE_AND             0b10001011
#define LCD_CG_RAM_MODE_TEXT            0b10001100

// OR together for ON modes
#define LCD_DISPLAY_MODES_ALL_OFF       0b10010000
#define LCD_DISPLAY_MODES_GRAPHICS_ON   0b10011000
#define LCD_DISPLAY_MODES_TEXT_ON       0b10010100
#define LCD_DISPLAY_MODES_CURSOR_ON     0b10010010
#define LCD_DISPLAY_MODES_CURSOR_BLINK  0b10010001

// Cursor Pattern Select
#define LCD_CURSOR_PATTERN_UNDERLINE    0b10100000
#define LCD_CURSOR_PATTERN_BLOCK        0b10100111

// Send Auto_XX Command, then block of data, then Auto_reset
#define LCD_DATA_AUTO_WRITE_SET         0b10110000
#define LCD_DATA_AUTO_READ_SET          0b10110001
#define LCD_DATA_AUTO_RESET             0b10110010

// Send R/W Then one byte Data
#define LCD_DATA_WRITE_AUTO_INCREMENT   0b11000000
#define LCD_DATA_READ_AUTO_INCREMENT    0b11000001
#define LCD_DATA_WRITE_NO_INCREMENT     0b11000100
#define LCD_DATA_READ_NO_INCREMENT      0b11000101

// Status Register Bits
#define LCD_STATUS_BUSY1    0x01
#define LCD_STATUS_BUSY2    0x02
#define LCD_STATUS_DARRDY   0x04
#define LCD_STATUS_DAWRDY   0x08
#define LCD_STATUS_CLR      0x20
#define LCD_STATUS_ERR      0x40
#define LCD_STATUS_BLINK    0x80

void lcd_write_command(u8 cmd);
void lcd_write_data(u8 data);
u8 lcd_read_data(void);
u8 lcd_read_status(void);
void lcd_init(void);
void lcd_bitmap(u8 *buff);

#endif
