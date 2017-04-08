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
#include "widgets.h"
#include "menu_functions.h"

//==============================================================================
//--------------------GLOBAL VARIABLES------------------------------------------
//==============================================================================
volatile u8 rot_enc_input = 0;



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
void osc_init(void)
{
    OSCTUN = 0; // 7.37 MHz
    CLKDIV = 0; // N1=2, N2=2
    PLLFBD = 63; // M=65
    // Fosc = 7.37*M/(N1*N2) = 119.7625 MHz
    // Fcy  = Fosc/2 = 59.88125 MIPS
    while (OSCCONbits.LOCK!=1){}; // Wait for PLL to lock
}

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
//--------------------COMMUNICATIONS HANDLER------------------------------------
//==============================================================================
void ecan_rx(u32 cid, u8 len, u8 rtr, u16 *data)
{

}



//==============================================================================
//--------------------MAIN LOOP-------------------------------------------------
//==============================================================================
int main(void)
{
    osc_init();
    led_init();
    rot_enc_init();
    lcd_init();
    ecan_init();
    tmr1_init(50); // 50 Hz == 20 ms ticks
    irq_init();

    read_widgets();

    // use function pointers to navigate through the menu;
    // default screen after bootup: home screen
    void* (*current_menu_fnc)(u8) = &menu_fnc_homescreen;

    while(1) {
        // clear lcd-buffer
        lcd_clrbuff();

        // latch input from rotary-encoder (changed during IRQs)
        u8 rot_enc_input_b = rot_enc_input;
        rot_enc_input = 0;

        // extra-long-press always gets you back to home-screen
        if ((rot_enc_input_b == C_ROT_EXTRALONGHOLD) || (current_menu_fnc == NULL)) {
            current_menu_fnc = &menu_fnc_homescreen;
            rot_enc_input_b = 0;
        }

        // call current menu-function and update next menu-function pointer
        current_menu_fnc = (*current_menu_fnc)(rot_enc_input_b);

        // copy lcd-buffer to lcd
        lcd_update();
        led_toggle();
    }
}
//==============================================================================
//==============================================================================
