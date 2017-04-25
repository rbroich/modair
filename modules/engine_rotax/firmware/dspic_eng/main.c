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

//==============================================================================
//--------------------GLOBAL VARIABLES------------------------------------------
//==============================================================================
extern u16 THIS_MODULE_ID;

#define PARAM_CNT 12
s_param_settings PARAM_LIST[PARAM_CNT] = {
    {.pid=0x0010, .name="BUS VOLT", .rate=0x20, .console_fnc_ptr=0},
    {.pid=0x0011, .name="EGT 1   ", .rate=0x20, .console_fnc_ptr=0},
    {.pid=0x0012, .name="EGT 2   ", .rate=0x20, .console_fnc_ptr=0},
    {.pid=0x0013, .name="RPM     ", .rate=0x20, .console_fnc_ptr=0},
    {.pid=0x0014, .name="ENG HRS ", .rate=0x20, .console_fnc_ptr=0},
    {.pid=0x0015, .name="ENG ON  ", .rate=0x20, .console_fnc_ptr=0},
    {.pid=0x0016, .name="MAINTAIN", .rate=0x20, .console_fnc_ptr=0},
    {.pid=0x0017, .name="FUEL LVL", .rate=0x20, .console_fnc_ptr=0},
    {.pid=0x0018, .name="H2O TEMP", .rate=0x20, .console_fnc_ptr=0},
    {.pid=0x0019, .name="RELAY   ", .rate=0x20, .console_fnc_ptr=0},
    {.pid=0x001A, .name="OD1 OUT ", .rate=0x20, .console_fnc_ptr=0},
    {.pid=0x001B, .name="OD2 OUT ", .rate=0x20, .console_fnc_ptr=0}
};


//==============================================================================
//--------------------INTERRUPTS------------------------------------------------
//==============================================================================
void __attribute__((interrupt, auto_psv)) _T1Interrupt(void)
{

    _T1IF = 0;  // clear the interrupt
}

void __attribute__((interrupt, auto_psv)) _CNInterrupt(void)
{
    fuelflow_irq();
    rpm_irq();
    _CNIF = 0;  // clear the interrupt
}

void __attribute__((interrupt, auto_psv)) _C1Interrupt(void)
{
    ecan_irq();
    _C1IF = 0;  // clear the interrupt
}

void __attribute__((interrupt, auto_psv)) _AD1Interrupt(void)
{
    analog_irq();
    _AD1IF = 0;  // clear the interrupt
}

void ecan_rx(u16 pid, u16 *data, u8 msg_type, u8 flags, u8 len)
{
    module_console_canrx(pid,data,msg_type,flags,len);


    // Handle incoming CAN messages; don't send messages from this IRQ
    u8 i;
    u8* dptr = (u8*)data;

    if ((msg_type==MT_REMOTE_CMD)&&(dptr[2]==MC_GET_NAME)&&(flags==0)&&(len==3))
        if ((data[0]==DPI_ALL_PARAMETERS)||(data[0]==THIS_MODULE_ID))
            for (i=0;i<PARAM_CNT;i++) // send all parameter ID names of this module
                ecan_tx_str(PARAM_LIST[i].pid, PARAM_LIST[i].name, MT_BROADCAST_NAME, 8);
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

    _C1IF = 0; // CAN1 Event Interrupt Flag
    _C1IP = 3; // third lowest priority
    _C1IE = ENABLE; // CAN1 Event Interrupt Enable

    _CNIF = 0; // ChangeNotification Flag
    _CNIP = 4; // fourth lowest priority level
    _CNIE = ENABLE; // change notification interrupt enable
}


//==============================================================================
//--------------------MAIN LOOP-------------------------------------------------
//==============================================================================
int main(void)
{
    OSCTUN = 0; // 7.37 MHz
    CLKDIV = 0; // N1=2, N2=2
    PLLFBD = 63; // M=65
    // Fosc = 7.37*M/(N1*N2) = 119.7625 MHz
    // Fcy  = Fosc/2 = 59.88125 MIPS
    while (OSCCONbits.LOCK!=1){}; // Wait for PLL to lock

    led_init();
    ecan_init();
    thermocouple_init();
    relay_init();
    fuelflow_init();
    rpm_init();
    analog_init();
    tmr1_init(50); // 50 Hz == 20 ms ticks
    irq_init();

    ecan_tx(0x1234, 'H' | 'e'<<8, 'l' | 'l'<<8, 'o' | ' '<<8, '1' | '2'<<8, MT_BROADCAST_NAME, 0, 8);

    while(1)
    {
        led_toggle();

        //delay_ms(220);

        module_console_process();

        //ecan_tx(0x1234, rpm_read(), thermocouple_read(0), analog_read_fuellevel(), analog_read_inputvoltage(), MT_BROADCAST_VALUE, 0, 8);
        // Bus Voltage
        // Thermocouple 1
        // Thermocouple 2
        // RPM
        // Engine Hours / Hobbs Meter
        // Engine On Time since started
        // Maintenance Timer
        // Fuel Level
        // Water Temperature

        // Relay Output
        // Open Drain 1 Output
        // Open Drain 2 Output

        // Fuel Flow Instantaneous
        // Fuel Flow Average since started
        // Time to Empty Tank (fuel endurance)
        // Range to Empty Tank (fuel range)
        // Fuel Burned
    }
}
