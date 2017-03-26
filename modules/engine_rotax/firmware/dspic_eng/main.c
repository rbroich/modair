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

//==============================================================================
//--------------------GLOBAL VARIABLES------------------------------------------
//==============================================================================




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



void ecan_rx(u32 cid, u8 len, u8 rtr, u16 *data)
{
    
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
    ecan_init();
    thermocouple_init();
    relay_init();
    fuelflow_init();
    rpm_init();
    analog_init();
    tmr1_init(50); // 50 Hz == 20 ms ticks
    irq_init();

    u16 tmp_data[4];
	tmp_data[0] = 0x0111; // data
	tmp_data[1] = 0x2222;
	tmp_data[2] = 0x5678;
	tmp_data[3] = 0xdddd;
    ecan_tx(0x19532408,8,0,tmp_data);

    while(1)
    {
        led_toggle();

        delay_ms(220);

        tmp_data[0] = rpm_read();
        tmp_data[1] = thermocouple_read(0);
        tmp_data[2] = analog_read_fuellevel();
        tmp_data[3] = analog_read_inputvoltage();
        ecan_tx(12,8,0,tmp_data);
        
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
