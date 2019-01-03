#include "xc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "config_bits.h"
#include "iopins.h"
#include "analog.h"
#include "fuellevel.h"
#include "busvoltage.h"
#include "watertemp.h"
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

// temporary settings record for flash memory read-modify-erase-write
s_settings tmp_settings;

//==============================================================================
//--------------------GLOBAL CONSTANTS------------------------------------------
//==============================================================================
// create a non-volatile program memory region 'nvmdata' at a page boundary,
// and store the user settings record there. Map the program memory region into
// data-memory using the PSV mechanism. The entire flash memory page needs to be
// erased before new settings can be written.
__attribute__((aligned(_FLASH_PAGE*2),space(psv),section(".nvmdata"))) const volatile s_settings settings = {
    .param = {
        {.pid=0xFF03, .name="Rotax582", .rate=0}, // MODULE
        {.pid=0x0091, .name="Air Pres", .rate=50}, // 1 Hz
        {.pid=0x0092, .name="QNH     ", .rate=0},  // 0 Hz
        {.pid=0x0093, .name="Alt FL  ", .rate=5},  // 10 Hz
        {.pid=0x0094, .name="Alt QNH ", .rate=5},  // 10 Hz
        {.pid=0x0095, .name="Air Temp", .rate=50}, // 1 Hz
        {.pid=0x0019, .name="RELAY   ", .rate=50},
        {.pid=0x001A, .name="OD1 OUT ", .rate=50},
        {.pid=0x001B, .name="OD2 OUT ", .rate=50},
        {.pid=0x0017, .name="FUEL LVL", .rate=10}, // 5 Hz
        {.pid=0x0010, .name="BUS VOLT", .rate=50}, // 1 Hz
        {.pid=0x0018, .name="H2O TEMP", .rate=10}, // 5 Hz
        {.pid=0x00A0, .name="HMC5883 ", .rate=50}, // 1 Hz
        {.pid=0x00A1, .name="MPU6050 ", .rate=50}  // 1 Hz
//    {.pid=0x0011, .name="EGT 1   ", .rate=12}, // ~4 Hz
//    {.pid=0x0012, .name="EGT 2   ", .rate=12}, // ~4 Hz
//    {.pid=0x0013, .name="RPM     ", .rate=25}, // 2 Hz
//    {.pid=0x0014, .name="ENG HRS ", .rate=50}, // 1 Hz
//    {.pid=0x0015, .name="ENG ON  ", .rate=50}, // 1 Hz
//    {.pid=0x0016, .name="MAINTAIN", .rate=50}, // 1 Hz
//    {.pid=0x001C, .name="FF INST ", .rate=10}, // 5 Hz
//    {.pid=0x001D, .name="FF AVE  ", .rate=10}, // 5 Hz
//    {.pid=0x001E, .name="FUEL END", .rate=25}, // 2 Hz
//    {.pid=0x001F, .name="FUEL RNG", .rate=25}, // 2 Hz
//    {.pid=0x0020, .name="FUEL USE", .rate=25} // 2 Hz
    },
    .fuellevel_rom = { // converts ADC value to fuel level in 0.01 liter
        .FLx = {0,360,700,975,1203,1400,1590,1750,1900,2030,2150,2250,2350,2435,2510,2600},
        .FLy = {0,300,647,982,1310,1637,2000,2353,2730,3099,3487,3845,4249,4626,4995,5488}
    },
    .watertemp_rom = { // converts ADC value to water temperature in 0.1 degrees
        .WTx = {0,41,72,126,239,360,638,1154,3320,3740,3892,3986,4095,4095,4095,4095},
        .WTy = {2000,1800,1500,1240,1000,840,650,440,-40,-190,-300,0,0,0,0,0}
    }
};

// register new parameter function handlers here; same order as settings.param[]
const s_param_fptr PARAM_CONST[PARAM_CNT] = {
    {.canrx_fnc_ptr=&module_ecanrx, .sendval_fnc_ptr=0,                   .menu_fnc_ptr=&menu_fnc_homescreen          }, // MODULE
    {.canrx_fnc_ptr=0,              .sendval_fnc_ptr=&bmp180_sendpres,    .menu_fnc_ptr=&bmp180_homescreen            }, // Air Pressure in Pa
    {.canrx_fnc_ptr=0,              .sendval_fnc_ptr=&bmp180_sendqnh,     .menu_fnc_ptr=&bmp180_editqnh               }, // QNH
    {.canrx_fnc_ptr=0,              .sendval_fnc_ptr=&bmp180_sendaltfl,   .menu_fnc_ptr=&bmp180_homescreen            }, // Altitude: Flight Level (i.e. QNH=101325 Pa)
    {.canrx_fnc_ptr=0,              .sendval_fnc_ptr=&bmp180_sendaltqnh,  .menu_fnc_ptr=&bmp180_homescreen            }, // Pressure Altitude: Based on user QNH
    {.canrx_fnc_ptr=0,              .sendval_fnc_ptr=&bmp180_sendtemp,    .menu_fnc_ptr=&bmp180_homescreen            }, // Outside Air Temperature in degrees C
    {.canrx_fnc_ptr=0,              .sendval_fnc_ptr=&iopins_sendrelay,   .menu_fnc_ptr=&iopins_relay_menu            }, // Relay Output
    {.canrx_fnc_ptr=0,              .sendval_fnc_ptr=&iopins_sendod1,     .menu_fnc_ptr=&iopins_od1_menu              }, // Open Drain 1 Output
    {.canrx_fnc_ptr=0,              .sendval_fnc_ptr=&iopins_sendod2,     .menu_fnc_ptr=&iopins_od2_menu              }, // Open Drain 2 Output
    {.canrx_fnc_ptr=0,              .sendval_fnc_ptr=&fuellevel_cntdwn,   .menu_fnc_ptr=&fuellevel_menu               }, // Fuel Level
    {.canrx_fnc_ptr=0,              .sendval_fnc_ptr=&busvoltage_cntdwn,  .menu_fnc_ptr=&busvoltage_menu              }, // Bus Voltage
    {.canrx_fnc_ptr=0,              .sendval_fnc_ptr=&watertemp_cntdwn,   .menu_fnc_ptr=&watertemp_menu               }, // Water Temperature
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

void __attribute__((interrupt, auto_psv)) _AD1Interrupt(void)
{
    analog_irq();
    _AD1IF = 0;  // clear the interrupt
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
    _AD1IF = 0; // ADC1 Event Interrupt Flag
    _AD1IP = 1; // lowest priority level
    _AD1IE = ENABLE; // ADC1 Event Interrupt Enable

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
            ecan_tx_str(settings.param[0].pid, (char*)settings.param[0].name, MT_BROADCAST_NAME, 8);

        if ((data[0]==DPI_ALL_PARAMETERS)||(data[0]==settings.param[0].pid)) // all parameters, or all parameters of this module
            for (i=1;i<PARAM_CNT;i++) // send all parameter ID names of this module
                ecan_tx_str(settings.param[i].pid, (char*)settings.param[i].name, MT_BROADCAST_NAME, 8);

        for (i=1;i<PARAM_CNT;i++)
            if (data[0]==settings.param[i].pid)
                ecan_tx_str(settings.param[i].pid, (char*)settings.param[i].name, MT_BROADCAST_NAME, 8);
    }
    if ((msg_type==MT_BROADCAST_NAME)&&(flags==ECAN_FLAGS_RTR)&&(len==0)) {
        for (i=0;i<PARAM_CNT;i++)
            if (data[0]==settings.param[i].pid)
                ecan_tx_str(settings.param[i].pid, (char*)settings.param[i].name, MT_BROADCAST_NAME, 8);
    }

    // Get Value
    if ( ((msg_type==MT_REMOTE_CMD)&&(dptr[2]==RC_GET_VALUE)&&(flags==0)&&(len==3)) ||
       ((msg_type==MT_BROADCAST_VALUE)&&(flags==ECAN_FLAGS_RTR)&&(len==0)) ) {
        for (i=1;i<PARAM_CNT;i++)
            if ((data[0]==settings.param[i].pid)&&(PARAM_CONST[i].sendval_fnc_ptr))
                PARAM_CONST[i].sendval_fnc_ptr(i);
    }

    // Remote Menu / Terminal Console
    if ((msg_type==MT_REMOTE_CMD)&&(dptr[2]==RC_CONSOLE_KEY)&&(flags==0)&&(len==4)) {
        for (i=0;i<PARAM_CNT;i++)
            if (data[0]==settings.param[i].pid) {
                static u8 current_menu_idx = 255;
                if ((current_menu_fnc==0)||(current_menu_idx!=i)) { // if not in sub-menu of previous
                    tmp_settings = settings; // copy settings for modifying
                    current_menu_fnc = PARAM_CONST[i].menu_fnc_ptr;
                }
                if (current_menu_fnc) // call console function
                    current_menu_fnc = (*current_menu_fnc)(i,dptr[3]);
                if (current_menu_fnc==0) // send EXIT if invalid or response==0
                    ecan_tx_console(settings.param[i].pid, 0);
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
    iopins_init();
    ecan_init();
    analog_init();
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
            if ((rate_cnt[i]==0)&&(settings.param[i].rate)) {
                if (PARAM_CONST[i].sendval_fnc_ptr) // if timeout fnc exists
                    PARAM_CONST[i].sendval_fnc_ptr(i);
                rate_cnt[i] = settings.param[i].rate; // reload countdown value
            }
    }
}
