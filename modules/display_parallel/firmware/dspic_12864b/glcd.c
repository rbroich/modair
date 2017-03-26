#include "glcd.h"

void lcd_write_command(u8 cmd)
{
    GL_RS = GL_RS_INSTR;
    delay_ns(20); // >10ns
    GL_E = 1;
    lcd_out(cmd); // >40ns before E falling edge
    delay_ns(320); // >160ns
    GL_E = 0;
    delay_us(144); // >72us
}

void lcd_write_data(u8 data)
{
    GL_RS = GL_RS_DATA;
    delay_ns(20); // >10ns
    GL_E = 1;
    lcd_out(data); // >40ns before E falling edge
    delay_ns(320); // >160ns
    GL_E = 0;
    delay_us(144); // >72us
}

void lcd_bitmap(u8 *buff)
{
    u8 y;
    u8 x;
    for (y=0;y<LCD_Y/2;y++) {
        lcd_write_command(LCD_EXT_RAM_ADDR | y);
        lcd_write_command(LCD_EXT_RAM_ADDR | 0x0);
        for (x=0;x<LCD_X/8;x+=2) {
            lcd_write_data(*buff++);
            lcd_write_data(*buff++);
        }
    }
    for (y=0;y<LCD_Y/2;y++) {
        lcd_write_command(LCD_EXT_RAM_ADDR | y);
        lcd_write_command(LCD_EXT_RAM_ADDR | 0x8);
        for (x=0;x<LCD_X/8;x+=2) {
            lcd_write_data(*buff++);
            lcd_write_data(*buff++);
        }
    }
}

void lcd_init(void)
{
    GL_PSB = GL_PSB_PARALLEL;
    lcd_ctrl_out();
    lcd_dir_out();
    GL_E = 0;

    delay_ms(250); // wait for > 40ms
    lcd_write_command(LCD_CMD_FUNCTION_SET | FUNCTION_SET_8BIT);
    lcd_write_command(LCD_CMD_FUNCTION_SET | FUNCTION_SET_8BIT | FUNCTION_SET_EXTENDED);
    lcd_write_command(LCD_CMD_FUNCTION_SET | FUNCTION_SET_8BIT | FUNCTION_SET_EXTENDED | FUNCTION_SET_G_ON);
}
