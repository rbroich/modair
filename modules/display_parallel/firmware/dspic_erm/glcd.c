#include "glcd.h"

u8 lcd_buff[LCD_X*LCD_Y/8];

u8 lcd_read_status(void)
{
    u8 status = 0;
    lcd_cd_cmd();
    lcd_dir_in();
    lcd_rw_rd();
    delay_ns(170); // access: 150ns
    lcd_in(status);
    lcd_rw_idle();
    delay_ns(50);
    return status; // input data is held for 50ns max
}

u8 lcd_read_data(void)
{
    u8 dval = 0;
    lcd_cd_data();
    lcd_dir_in();
    lcd_rw_rd();
    delay_ns(170); // access: 150ns
    lcd_in(dval);
    lcd_rw_idle();
    delay_ns(50);
    return dval; // input data is held for 50ns max
}

void lcd_write_command(u8 cmd)
{
    lcd_cd_cmd();
    lcd_out(cmd);
    lcd_dir_out();
    lcd_rw_wr();
    delay_ns(120); // data/CD setup: 100ns
    lcd_rw_idle();
    delay_ns(20); // output data hold: 10ns min
}

void lcd_write_data(u8 data)
{
    lcd_cd_data();
    lcd_out(data);
    lcd_dir_out();
    lcd_rw_wr();
    delay_ns(120); // data/CD setup: 100ns
    lcd_rw_idle();
    delay_ns(20); // output data hold: 10ns min
}

void lcd_init(void)
{
    lcd_ctrl_out();
    lcd_rw_idle();

    while ((lcd_read_status()&0x03) != 0x03); // Not busy: STA0=1 and STA1=1
    lcd_write_data(0x00);
    while ((lcd_read_status()&0x03) != 0x03); // Not busy: STA0=1 and STA1=1
    lcd_write_data(0x00);
    while ((lcd_read_status()&0x03) != 0x03); // Not busy: STA0=1 and STA1=1
    lcd_write_command(LCD_GRAPHIC_HOME_ADDRESS_SET);

    while ((lcd_read_status()&0x03) != 0x03); // Not busy: STA0=1 and STA1=1
    lcd_write_data(0x20);
    while ((lcd_read_status()&0x03) != 0x03); // Not busy: STA0=1 and STA1=1
    lcd_write_data(0x00);
    while ((lcd_read_status()&0x03) != 0x03); // Not busy: STA0=1 and STA1=1
    lcd_write_command(LCD_GRAPHIC_AREA_SET);

    while ((lcd_read_status()&0x03) != 0x03); // Not busy: STA0=1 and STA1=1
    lcd_write_command(LCD_CG_ROM_MODE_OR);

    while ((lcd_read_status()&0x03) != 0x03); // Not busy: STA0=1 and STA1=1
    lcd_write_command(LCD_DISPLAY_MODES_GRAPHICS_ON);
}

void lcd_update() // copy display buffer to the LCD
{
    lcd_init();
    
    while ((lcd_read_status()&0x03) != 0x03); // Not busy: STA0=1 and STA1=1
    lcd_write_data(0x00);
    while ((lcd_read_status()&0x03) != 0x03); // Not busy: STA0=1 and STA1=1
    lcd_write_data(0x00);
    while ((lcd_read_status()&0x03) != 0x03); // Not busy: STA0=1 and STA1=1
    lcd_write_command(LCD_ADDRESS_POINTER_SET);

    while ((lcd_read_status()&0x03) != 0x03); // Not busy: STA0=1 and STA1=1
    lcd_write_command(LCD_DATA_AUTO_WRITE_SET);

    int k;
    for(k=0;k<LCD_Y*LCD_X/8;k++)
    {
        while ((lcd_read_status()&0x08) != 0x08); // While not STA3
        lcd_write_data(lcd_buff[k]);
    }
    while ((lcd_read_status()&0x08) != 0x08); // While not STA3
    lcd_write_command(LCD_DATA_AUTO_RESET);
}

void lcd_clrbuff() // clear the display buffer
{
    u16 i;
    for (i=0;i<LCD_X*LCD_Y/8;i++)
        lcd_buff[i] = 0;
}

void lcd_setpixel(u8 x, u8 y, u8 color)
{
    if ((x>=LCD_X)||(y>=LCD_Y))
        return;
    if (color == LCD_BLACK)
        lcd_buff[y*LCD_X/8 + ((x & 0xF8) >> 3)] |= (1<<(7-(x&0x07))); // set bit
    if (color == LCD_WHITE)
        lcd_buff[y*LCD_X/8 + ((x & 0xF8) >> 3)] &= (~(1<<(7-(x&0x07)))); // clear bit
}