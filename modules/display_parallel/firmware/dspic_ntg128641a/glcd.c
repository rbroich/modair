#include "glcd.h"

u8 lcd_buff[LCD_X*LCD_Y/8];

void lcd_out(u8 val)
{
    GL_RS = GL_RS_DATA;
    delay_ns(140); // RS/CS1/CS2 setup time (140 ns)
    GL_E = 1;
    lcd_data_out(val); // data setup time (200 ns)
    delay_ns(500); // E high level width (500 ns)
    GL_E = 0;
    delay_us(60); // wait while LCD is busy
}

void lcd_cmd(u8 val)
{
    GL_RS = GL_RS_INSTR;
    delay_ns(140); // address setup time (RS/CS1/CS2)
    GL_E = 1;
    lcd_data_out(val); // data setup time (200 ns)
    delay_ns(500); // E high level width (500 ns)
    GL_E = 0;
    delay_us(60); // wait while LCD is busy
}

void lcd_fill(u8 color)
{
    u8 i,j;
    for (j=0;j<8;j++) {
        lcd_cmd(LCD_SET_PAGE + j); // set page j
        if (color == LCD_BLACK) {
            for (i=0;i<64;i++)
                lcd_out(0xFF);
        }
        if (color == LCD_WHITE) {
            for (i=0;i<64;i++)
                lcd_out(0x00);
        }
    }
}

void lcd_init(void)
{
    lcd_dir_out(); // set pin directions to OUTPUT
    lcd_ctrl_out();
    delay_us(20); // reset time: min 1 us

    GL_CS1 = 1;
    GL_CS2 = 1;
    lcd_data_out(0x00);
    GL_E = 0;
    GL_RS = 0;
    delay_us(20);
    lcd_cmd(LCD_OFF); // display off
    lcd_cmd(LCD_START_LINE | 0); // start line = 0
    lcd_cmd(LCD_SET_ADDR | 0); // set addr = 0
    lcd_cmd(LCD_SET_PAGE | 0); // set page = 0
    lcd_fill(0);
    lcd_cmd(LCD_ON); // display on
    GL_CS1 = 0;
    GL_CS2 = 0;
}

void lcd_update() // copy display buffer to the LCD
{
    u8* buff = lcd_buff;
    u8 i,j;
    GL_CS1 = 1;
    GL_CS2 = 1;
    lcd_cmd(LCD_START_LINE | 0); // start line = 0
    for (i=0;i<8;i++) {
        lcd_cmd(LCD_SET_PAGE | i); // set page = i
        lcd_cmd(LCD_SET_ADDR | 0); // set addr = 0
        GL_CS2 = 0;
        for (j=0;j<64;j++)
            lcd_out(*buff++);
        GL_CS1 = 0;
        GL_CS2 = 1;
        for (j=0;j<64;j++)
            lcd_out(*buff++);
        GL_CS1 = 1;
    }
    lcd_cmd(LCD_ON); // display on
    GL_CS1 = 0;
    GL_CS2 = 0;
}

void lcd_clrbuff() // clear the display buffer
{
    u16 i;
    for (i=0;i<LCD_X*LCD_Y/8;i++)
        lcd_buff[i] = 0;
}

void lcd_setpixel(u8 x, u8 y, u8 color) // set/clr a bit in the display buffer
{
    if ((x>=LCD_X)||(y>=LCD_Y))
        return;
    u8 xi = LCD_Y-1-y;
    u8 yi = x;

    u16 buff_offset = ((yi&0xF8)<<4)+xi;
    if (color == LCD_BLACK)
        lcd_buff[buff_offset] |= (1<<(yi&0x07)); // set bit
    if (color == LCD_WHITE)
        lcd_buff[buff_offset] &= (~(1<<(yi&0x07))); // clear bit
}

