#include "xc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "config_bits.h"
#include "led.h"
#include "ecan_mod.h"
#include "relay.h"
#include "rpm.h"
#include "fuelflow.h"
#include "analog.h"
#include "thermocouple.h"
#include "modair_bus.h"
#include "params.h"
#include "module_console.h"
#include "fuellevel.h"
#include "busvoltage.h"
#include "enginehours.h"
#include "opendrain.h"
#include "watertemp.h"

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
    {.pid=0xFF02, .name="Rotax582", .rate=0}, // MODULE
    {.pid=0x0010, .name="BUS VOLT", .rate=50}, // 1 Hz
    {.pid=0x0011, .name="EGT 1   ", .rate=12}, // ~4 Hz
    {.pid=0x0012, .name="EGT 2   ", .rate=12}, // ~4 Hz
    {.pid=0x0013, .name="RPM     ", .rate=25}, // 2 Hz
    {.pid=0x0014, .name="ENG HRS ", .rate=50}, // 1 Hz
    {.pid=0x0015, .name="ENG ON  ", .rate=50}, // 1 Hz
    {.pid=0x0016, .name="MAINTAIN", .rate=50}, // 1 Hz
    {.pid=0x0017, .name="FUEL LVL", .rate=10}, // 5 Hz
    {.pid=0x0018, .name="H2O TEMP", .rate=10}, // 5 Hz
    {.pid=0x0019, .name="RELAY   ", .rate=0},
    {.pid=0x001A, .name="OD1 OUT ", .rate=0},
    {.pid=0x001B, .name="OD2 OUT ", .rate=0},
    {.pid=0x001C, .name="FF INST ", .rate=10}, // 5 Hz
    {.pid=0x001D, .name="FF AVE  ", .rate=10}, // 5 Hz
    {.pid=0x001E, .name="FUEL END", .rate=25}, // 2 Hz
    {.pid=0x001F, .name="FUEL RNG", .rate=25}, // 2 Hz
    {.pid=0x0020, .name="FUEL USE", .rate=25}  // 2 Hz
};

const s_param_const PARAM_CONST[PARAM_CNT] = {
    {.canrx_fnc_ptr=&module_ecanrx, .cntdwn_fnc_ptr=0,                    .menu_fnc_ptr=&menu_fnc_homescreen          }, // MODULE
    {.canrx_fnc_ptr=0,              .cntdwn_fnc_ptr=&busvoltage_cntdwn,   .menu_fnc_ptr=&busvoltage_fnc_homescreen    }, // Bus Voltage
    {.canrx_fnc_ptr=0,              .cntdwn_fnc_ptr=&thermocouple_cntdwn, .menu_fnc_ptr=&thermocouple_fnc_homescreen  }, // Thermocouple 1
    {.canrx_fnc_ptr=0,              .cntdwn_fnc_ptr=&thermocouple_cntdwn, .menu_fnc_ptr=&thermocouple_fnc_homescreen  }, // Thermocouple 2
    {.canrx_fnc_ptr=0,              .cntdwn_fnc_ptr=&rpm_cntdwn,          .menu_fnc_ptr=0                             }, // RPM
    {.canrx_fnc_ptr=0,              .cntdwn_fnc_ptr=&enginehours_cntdwn,  .menu_fnc_ptr=&enginehours_fnc_homescreen   }, // Engine Hours / Hobbs Meter
    {.canrx_fnc_ptr=0,              .cntdwn_fnc_ptr=0,                    .menu_fnc_ptr=0                             }, // Engine On Time since started
    {.canrx_fnc_ptr=0,              .cntdwn_fnc_ptr=0,                    .menu_fnc_ptr=0                             }, // Maintenance Timer
    {.canrx_fnc_ptr=0,              .cntdwn_fnc_ptr=&fuellevel_cntdwn,    .menu_fnc_ptr=&fuellevel_fnc_homescreen     }, // Fuel Level
    {.canrx_fnc_ptr=0,              .cntdwn_fnc_ptr=&watertemp_cntdwn,    .menu_fnc_ptr=0                             }, // Water Temperature
    {.canrx_fnc_ptr=0,              .cntdwn_fnc_ptr=0,                    .menu_fnc_ptr=0                             }, // Relay Output
    {.canrx_fnc_ptr=0,              .cntdwn_fnc_ptr=0,                    .menu_fnc_ptr=0                             }, // Open Drain 1 Output
    {.canrx_fnc_ptr=0,              .cntdwn_fnc_ptr=0,                    .menu_fnc_ptr=0                             }, // Open Drain 2 Output
    {.canrx_fnc_ptr=0,              .cntdwn_fnc_ptr=0,                    .menu_fnc_ptr=0                             }, // Fuel Flow Instantaneous
    {.canrx_fnc_ptr=0,              .cntdwn_fnc_ptr=0,                    .menu_fnc_ptr=0                             }, // Fuel Flow Average since started
    {.canrx_fnc_ptr=0,              .cntdwn_fnc_ptr=0,                    .menu_fnc_ptr=0                             }, // Time to Empty Tank (fuel endurance)
    {.canrx_fnc_ptr=0,              .cntdwn_fnc_ptr=0,                    .menu_fnc_ptr=0                             }, // Range to Empty Tank (fuel range)
    {.canrx_fnc_ptr=0,              .cntdwn_fnc_ptr=0,                    .menu_fnc_ptr=0                             }  // Fuel Burned
};

const s_fuelcal fuellevel_rom = { // converts ADC value to fuel level in 0.01 liter
    .FLx = {0,360,700,975,1203,1400,1590,1750,1900,2030,2150,2250,2350,2435,2510,2600},
    .FLy = {0,300,647,982,1310,1637,2000,2353,2730,3099,3487,3845,4249,4626,4995,5488}
};

const s_watertemp watertemp_rom = { // converts ADC value to water temperature in 0.1 degrees
    .WTx = {0,41,72,126,239,360,638,1154,3320,3740,3892,3986,4095,4095,4095,4095},
    .WTy = {2000,1800,1500,1240,1000,840,650,440,-40,-190,-300,-400,-500,-500,-500,-500}
};

const u16 rmp_mul = 20; // CNTs per 0.5s => 2.0 (cnts/sec) * 60.0 (cnts/min) / 6.0 (rotax582 setting?)
const float engine_hobbs = 1202.4; // hours
const u8 maintainance_date[3] = {28,04,18}; // dd,mm,20yy
const float maintanance_hobbs = 1250.0;

//==============================================================================
//--------------------INTERRUPTS------------------------------------------------
//==============================================================================
void __attribute__((interrupt, auto_psv)) _T1Interrupt(void)
{
    u8 i;
    for (i=1;i<PARAM_CNT;i++)
        if (rate_cnt[i])
            rate_cnt[i]--;
    enginehours_tmr_irq();
    rpm_tmr_irq();
    _T1IF = 0;  // clear the interrupt
}

void __attribute__((interrupt, auto_psv)) _CNInterrupt(void)
{
    fuelflow_cn_irq();
    rpm_cn_irq();
    _CNIF = 0;  // clear the interrupt
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

void irq_init(void)
{
    _AD1IF = 0; // ADC1 Event Interrupt Flag
    _AD1IP = 1; // lowest priority level
    _AD1IE = ENABLE; // ADC1 Event Interrupt Enable

    _T1IF = 0; // Timer1 Flag
    _T1IP = 2; // second lowest priority
    _T1IE = ENABLE; // timer1 interrupt enable

    _CNIF = 0; // ChangeNotification Flag
    _CNIP = 3; // third lowest priority level
    _CNIE = ENABLE; // change notification interrupt enable
}

//==============================================================================
//--------------------MODULE CAN PROCESS----------------------------------------
//==============================================================================
void module_ecanrx(u8 idx, u16 pid, u16 *data, u8 msg_type, u8 flags, u8 len)
{
    u8 i;
    u8* dptr = (u8*)data;
    if (idx) return; // expect idx==0 for MODULE

    if ((msg_type==MT_REMOTE_CMD)&&(dptr[2]==MC_GET_NAME)&&(flags==0)&&(len==3)) {
        if (data[0]==DPI_ALL_MODULES) // send this module name
            ecan_tx_str(PARAM_LIST[0].pid, (char*)PARAM_LIST[0].name, MT_BROADCAST_NAME, 8);

        if ((data[0]==DPI_ALL_PARAMETERS)||(data[0]==PARAM_LIST[0].pid)) // all parameters, or all parameters of this module
            for (i=1;i<PARAM_CNT;i++) // send all parameter ID names of this module
                ecan_tx_str(PARAM_LIST[i].pid, (char*)PARAM_LIST[i].name, MT_BROADCAST_NAME, 8);

        for (i=1;i<PARAM_CNT;i++)
            if (data[0]==PARAM_LIST[i].pid)
                ecan_tx_str(PARAM_LIST[i].pid, (char*)PARAM_LIST[i].name, MT_BROADCAST_NAME, 8);
    }
    if ((msg_type==MT_REMOTE_CMD)&&(dptr[2]==MC_GET_VALUE)&&(flags==0)&&(len==3)) {
        for (i=1;i<PARAM_CNT;i++)
            if ((data[0]==PARAM_LIST[i].pid)&&(PARAM_CONST[i].cntdwn_fnc_ptr))
                PARAM_CONST[i].cntdwn_fnc_ptr(i);
    }

    if ((msg_type==MT_REMOTE_CMD)&&(dptr[2]==MC_TERMINAL_KEY)&&(flags==0)&&(len==4))  {
        for (i=0;i<PARAM_CNT;i++)
            if (data[0]==PARAM_LIST[i].pid) {
                static u8 current_menu_idx = 255;
                if ((current_menu_fnc==NULL)||(current_menu_idx!=i)) // if not in sub-menu of previous
                    current_menu_fnc = PARAM_CONST[i].menu_fnc_ptr;
                if (current_menu_fnc) // call console function
                    current_menu_fnc = (*current_menu_fnc)(i,dptr[3]);
                if (current_menu_fnc==NULL) // send EXIT if invalid or response==0
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
    opendrain_init();
    ecan_init();
    enginehours_init();
    thermocouple_init();
    relay_init();
    fuelflow_init();
    rpm_init();
    analog_init();
    tmr1_init(50); // 50 Hz == 20 ms ticks
    irq_init();

    while(1)
    {
        // handle CAN receive messages here
        ecan_process();

        // handle TMR timeout function calls here
        for (i=1;i<PARAM_CNT;i++)
            if ((rate_cnt[i]==0)&&(PARAM_LIST[i].rate)) {
                if (PARAM_CONST[i].cntdwn_fnc_ptr) // if timeout fnc exists
                    PARAM_CONST[i].cntdwn_fnc_ptr(i);
                rate_cnt[i] = PARAM_LIST[i].rate; // reload countdown value
            }
    }
}
