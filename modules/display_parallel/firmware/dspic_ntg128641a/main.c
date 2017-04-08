#include "xc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "config_bits.h"
#include "led.h"
#include "rot_enc.h"
#include "glcd.h"
#include "glcd_lib.h"
#include "ecan_mod.h"

//==============================================================================
//--------------------GLOBAL VARIABLES------------------------------------------
//==============================================================================
extern volatile u8 rot_enc_input;



//==============================================================================
//--------------------INTERRUPTS------------------------------------------------
//==============================================================================
void __attribute__((interrupt, auto_psv)) _T1Interrupt(void)
{
    rot_enc_tmr();
    _T1IF = 0;  // clear the interrupt
}

void __attribute__((interrupt, auto_psv)) _CNInterrupt(void)
{
    rot_enc_irq();
    _CNIF = 0;  // clear the interrupt
}

void __attribute__((interrupt, auto_psv)) _C1Interrupt(void)
{
    ecan_irq();
    _C1IF = 0;  // clear the interrupt
}

//==============================================================================
//--------------------INIT FUNCTIONS--------------------------------------------
//==============================================================================
void tmr1_init(u16 freq_hz)
{
    T1CON = 0b1000000000110000; // TMR1 on, 1:256 prescale, Fosc/2
    PR1 = F_CY/256/freq_hz;
}

void irq_init(void)
{
    _T1IF = 0; // Timer1 Flag
    _T1IP = 2; // second lowest priority level
    _T1IE = ENABLE; // timer1 interrupt enable

    _CNIF = 0; // ChangeNotification Flag
    _CNIP = 1; // lowest priority level
    _CNIE = ENABLE; // change notification interrupt enable

    _C1IF = 0; // CAN1 Event Interrupt Flag
    _C1IP = 3; // third lowest priority
    _C1IE = ENABLE; // CAN1 Event Interrupt Enable
}

//==============================================================================
//--------------------MAIN LOOP-------------------------------------------------
//==============================================================================



u32 cid_rx = 0x01234567;
u8 len_rx = 5;
u8 rtr_rx = 1;
u16 data_rx[4] = {0,0,0,0};
void ecan_rx(u32 cid, u8 len, u8 rtr, u16 *data)
{
    cid_rx = cid;
    len_rx = len;
    rtr_rx = rtr;
    data_rx[0] = data[0];
    data_rx[1] = data[1];
    data_rx[2] = data[2];
    data_rx[3] = data[3];
}



int main(void)
{
    OSCTUN = 0; // 7.37 MHz
    CLKDIV = 0; // N1=2, N2=2
    PLLFBD = 63; // M=65
    // Fosc = 7.37*M/(N1*N2) = 119.7625 MHz
    // Fcy  = Fosc/2 = 59.88125 MIPS
    while (OSCCONbits.LOCK!=1){}; // Wait for PLL to lock

    led_init();
    rot_enc_init();
    lcd_init();
    ecan_init();
    tmr1_init(50); // 50 Hz == 20 ms ticks
    irq_init();

    //C1CTRL1bits.REQOP = 0b010; // enter loopback mode
    //while(C1CTRL1bits.OPMODE != 0b010); // wait for loopback mode

    u16 tmp_data[4];
	tmp_data[0] = 0x0123; // data
	tmp_data[1] = 0x4567;
	tmp_data[2] = 0x89ab;
	tmp_data[3] = 0xcdef;
    ecan_tx(0xF9532408,8,0,tmp_data);

    u8 i = 10;
    while(1)
    {
        if (rot_enc_input) {
            switch(rot_enc_input) {
                case C_ROT_INC:
                    i++;
                    break;
                case C_ROT_DEC:
                    i--;
                    break;
                case C_ROT_PUSH:
                    i = 0;
                    break;
                case C_ROT_HOLD:
                    i = 10;
                    break;
                case C_ROT_LONGHOLD:
                    i = 20;
                    break;
                case C_ROT_EXTRALONGHOLD:
                    i = 30;
                    break;
            }
            rot_enc_input = 0;
        }

        lcd_clrbuff();
        
        char tmp_str[16];
        mprint_int(tmp_str, cid_rx>>16, 16, 4);
        LCD_string(tmp_str, 20, 20, GLCD_ROTATE_0, GLCD_FONT_5x7, LCD_BLACK, 32);
        mprint_int(tmp_str, cid_rx&0xFFFF, 16, 4);
        LCD_string(tmp_str, 44, 20, GLCD_ROTATE_0, GLCD_FONT_5x7, LCD_BLACK, 32);
        
        LCD_string("Len:", 80, 20, GLCD_ROTATE_0, GLCD_FONT_5x7, LCD_BLACK, 32);
        mprint_int(tmp_str, len_rx, 10, 1);
        LCD_string(tmp_str, 104, 20, GLCD_ROTATE_0, GLCD_FONT_5x7, LCD_BLACK, 32);

        u8 j;
        for (j=0;j<4;j++) {
            mprint_int(tmp_str, data_rx[j], 16, 4);
            LCD_string(tmp_str, 10+j*28, 36, GLCD_ROTATE_0, GLCD_FONT_5x7, LCD_BLACK, 32);
        }

        LCD_rect(0, 0, LCD_X-1, LCD_Y-1, LCD_BLACK, 0);
        if (rtr_rx)
            LCD_circle(100, 50, 5, LCD_BLACK);
        LCD_string("ModAir", i, 10, GLCD_ROTATE_0, GLCD_FONT_5x7, LCD_BLACK, 32);

        lcd_update();

        led_toggle();
        //delay_ms(100);
    }
}
