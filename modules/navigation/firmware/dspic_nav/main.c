#include "xc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "config_bits.h"
#include "led.h"
#include "ecan_mod.h"
#include "modair_bus.h"
#include "params.h"
#include "module_console.h"
#include "i2c.h"
#include "bmp180_driv.h"
#include "mpu6050_driv.h"
#include "hmc5883_driv.h"

//==============================================================================
//--------------------FUNCTION PROTOTYPES---------------------------------------
//==============================================================================
void module_ecanrx(u8 idx, u16 pid, u16 *data, u8 msg_type, u8 flags, u8 len);

//==============================================================================
//--------------------GLOBAL VARIABLES------------------------------------------
//==============================================================================
//s_param_settings PARAM_LIST[PARAM_CNT];
u16 rate_cnt[PARAM_CNT];

// use function pointers to navigate through the module menu;
void* (*current_menu_fnc)(u8,u8) = 0;

//==============================================================================
//--------------------GLOBAL CONSTANTS------------------------------------------
//==============================================================================
// TODO: make this constant is stored in flash (such that it is rewritable)
__attribute__((aligned(FLASH_PAGE_SIZE))) const s_param_settings PARAM_LIST[PARAM_CNT] = {
    {.pid=0xFF03, .name="Rotax582", .rate=0}, // MODULE
    {.pid=0x0091, .name="Air Pres", .rate=50}, // 1 Hz
    {.pid=0x0092, .name="QNH     ", .rate=0},  // 0 Hz
    {.pid=0x0093, .name="Alt FL  ", .rate=5},  // 10 Hz
    {.pid=0x0094, .name="Alt QNH ", .rate=5},  // 10 Hz
    {.pid=0x0095, .name="Air Temp", .rate=50}, // 1 Hz
    {.pid=0x00A0, .name="HMC5883 ", .rate=50}, // 1 Hz
    {.pid=0x00A1, .name="MPU6050 ", .rate=50}  // 1 Hz
};

const s_param_const PARAM_CONST[PARAM_CNT] = {
    {.canrx_fnc_ptr=&module_ecanrx, .sendval_fnc_ptr=0,                   .menu_fnc_ptr=&menu_fnc_homescreen          }, // MODULE
    {.canrx_fnc_ptr=0,              .sendval_fnc_ptr=&bmp180_sendpres,    .menu_fnc_ptr=&bmp180_homescreen            }, // Air Pressure in Pa
    {.canrx_fnc_ptr=0,              .sendval_fnc_ptr=&bmp180_sendqnh,     .menu_fnc_ptr=&bmp180_editqnh               }, // QNH
    {.canrx_fnc_ptr=0,              .sendval_fnc_ptr=&bmp180_sendaltfl,   .menu_fnc_ptr=&bmp180_homescreen            }, // Altitude: Flight Level (i.e. QNH=101325 Pa)
    {.canrx_fnc_ptr=0,              .sendval_fnc_ptr=&bmp180_sendaltqnh,  .menu_fnc_ptr=&bmp180_homescreen            }, // Pressure Altitude: Based on user QNH
    {.canrx_fnc_ptr=0,              .sendval_fnc_ptr=&bmp180_sendtemp,    .menu_fnc_ptr=&bmp180_homescreen            }, // Outside Air Temperature in degrees C
    {.canrx_fnc_ptr=0,              .sendval_fnc_ptr=0,                   .menu_fnc_ptr=&hmc5883_homescreen           },
    {.canrx_fnc_ptr=0,              .sendval_fnc_ptr=0,                   .menu_fnc_ptr=&mpu6050_homescreen           }
};

//==============================================================================
//--------------------INTERRUPTS------------------------------------------------
//==============================================================================
void __attribute__((interrupt, auto_psv)) _T1Interrupt(void)
{
    u8 i;
    for (i=1;i<PARAM_CNT;i++)
        if (rate_cnt[i])
            rate_cnt[i]--;
    _T1IF = 0;  // clear the interrupt
}

void __attribute__((interrupt, auto_psv)) _T2Interrupt(void)
{
    bmp180_tmr_irq();
    _T2IF = 0;  // clear the interrupt
}

//==============================================================================
//--------------------INIT FUNCTIONS--------------------------------------------
//==============================================================================
void tmr1_init(u16 freq_hz)
{
    T1CON = 0b1000000000110000; // TMR1 on, 1:256 prescale, Fosc/2
    PR1 = F_CY/256/freq_hz;
}

void tmr2_init(u16 freq_hz)
{
    T2CON = 0b1000000000110000; // TMR2 on, 1:256 prescale, 2x indept 16-bit, Fosc/2
    PR2 = F_CY/256/freq_hz;
}

void irq_init(void)
{
    _T2IF = 0; // Timer1 Flag
    _T2IP = 1; // lowest priority level
    _T2IE = ENABLE; // timer1 interrupt enable

    _T1IF = 0; // Timer1 Flag
    _T1IP = 3; // third lowest priority level
    _T1IE = ENABLE; // timer1 interrupt enable
}

//==============================================================================
//--------------------MODULE CAN PROCESS----------------------------------------
//==============================================================================
void module_ecanrx(u8 idx, u16 pid, u16 *data, u8 msg_type, u8 flags, u8 len)
{
    u8 i;
    u8* dptr = (u8*)data;
    if (idx) return; // expect idx==0 for MODULE

    // Get Name
    if ((msg_type==MT_REMOTE_CMD)&&(dptr[2]==RC_GET_NAME)&&(flags==0)&&(len==3)) {
        if (data[0]==DPI_ALL_MODULES) // send this module name
            ecan_tx_str(PARAM_LIST[0].pid, (char*)PARAM_LIST[0].name, MT_BROADCAST_NAME, 8);

        if ((data[0]==DPI_ALL_PARAMETERS)||(data[0]==PARAM_LIST[0].pid)) // all parameters, or all parameters of this module
            for (i=1;i<PARAM_CNT;i++) // send all parameter ID names of this module
                ecan_tx_str(PARAM_LIST[i].pid, (char*)PARAM_LIST[i].name, MT_BROADCAST_NAME, 8);

        for (i=1;i<PARAM_CNT;i++)
            if (data[0]==PARAM_LIST[i].pid)
                ecan_tx_str(PARAM_LIST[i].pid, (char*)PARAM_LIST[i].name, MT_BROADCAST_NAME, 8);
    }
    if ((msg_type==MT_BROADCAST_NAME)&&(flags==ECAN_FLAGS_RTR)&&(len==0)) {
        for (i=0;i<PARAM_CNT;i++)
            if (data[0]==PARAM_LIST[i].pid)
                ecan_tx_str(PARAM_LIST[i].pid, (char*)PARAM_LIST[i].name, MT_BROADCAST_NAME, 8);
    }

    // Get Value
    if ( ((msg_type==MT_REMOTE_CMD)&&(dptr[2]==RC_GET_VALUE)&&(flags==0)&&(len==3)) ||
       ((msg_type==MT_BROADCAST_VALUE)&&(flags==ECAN_FLAGS_RTR)&&(len==0)) ) {
        for (i=1;i<PARAM_CNT;i++)
            if ((data[0]==PARAM_LIST[i].pid)&&(PARAM_CONST[i].sendval_fnc_ptr))
                PARAM_CONST[i].sendval_fnc_ptr(i);
    }

    // Remote Menu / Terminal Console
    if ((msg_type==MT_REMOTE_CMD)&&(dptr[2]==RC_CONSOLE_KEY)&&(flags==0)&&(len==4)) {
        for (i=0;i<PARAM_CNT;i++)
            if (data[0]==PARAM_LIST[i].pid) {
                static u8 current_menu_idx = 255;
                if ((current_menu_fnc==0)||(current_menu_idx!=i)) // if not in sub-menu of previous
                    current_menu_fnc = PARAM_CONST[i].menu_fnc_ptr;
                if (current_menu_fnc) // call console function
                    current_menu_fnc = (*current_menu_fnc)(i,dptr[3]);
                if (current_menu_fnc==0) // send EXIT if invalid or response==0
                    ecan_tx_console(PARAM_LIST[i].pid, 0);
                current_menu_idx = i;
                break;
            }
    }
}

//==============================================================================
//--------------------MAIN LOOP-------------------------------------------------
//==============================================================================
int main(void)
{
    u8 i;
    OSCTUN = 0; // 7.37 MHz
    CLKDIV = 0; // N1=2, N2=2
    PLLFBD = 63; // M=65
    // Fosc = 7.37*M/(N1*N2) = 119.7625 MHz
    // Fcy  = Fosc/2 = 59.88125 MIPS
    while (OSCCONbits.LOCK!=1){}; // Wait for PLL to lock

    // Call Parameter Init functions
    led_init();
    ecan_init();
    i2c_init();
    bmp180_init(BMP180_ULTRALOWPOWER);
    mpu6050_init();
    hmc5883_init();
    tmr1_init(50); // 50 Hz == 20 ms ticks
    tmr2_init(1000); // 1000 Hz == 1 ms ticks
    irq_init();

    while(1)
    {
        // handle CAN receive messages here
        ecan_process();

        // process pressure sensor readings
        bmp180_process();

        // handle TMR timeout function calls here
        for (i=1;i<PARAM_CNT;i++)
            if ((rate_cnt[i]==0)&&(PARAM_LIST[i].rate)) {
                if (PARAM_CONST[i].sendval_fnc_ptr) // if timeout fnc exists
                    PARAM_CONST[i].sendval_fnc_ptr(i);
                rate_cnt[i] = PARAM_LIST[i].rate; // reload countdown value
            }
    }
}
