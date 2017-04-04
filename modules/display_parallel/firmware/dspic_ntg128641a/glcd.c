#include "glcd.h"

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
    for (j=0;j<8;j++)
    {
        lcd_cmd(LCD_SET_PAGE + j); // set page j
        if (color) {
            for (i=0;i<64;i++)
                lcd_out(0xFF);
        } else {
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

void lcd_bitmap(u8 *buff)
{
    u8 i,j;
    lcd_cmd(LCD_START_LINE | 0); // start line = 0
    for (i=0;i<8;i++) {
        lcd_cmd(LCD_SET_PAGE | i); // set page = i
        lcd_cmd(LCD_SET_ADDR | 0); // set addr = 0
        for (j=0;j<64;j++)
            lcd_out(*buff++);
    }
    lcd_cmd(LCD_ON); // display on
}
