#include "analog.h"

u16 adc_val_vin;
u16 adc_val_ntc;
u16 adc_val_fl;

void analog_irq(void)
{
    adc_val_vin = ADC1BUF0; // AN0 == VIN
    adc_val_ntc = ADC1BUF1; // AN1 == Water Temperature (NTC)
    adc_val_fl = ADC1BUF2; // AN5 == Fuel Level (FL)
}

void analog_init(void)
{
    VIN_ANa = ANALOG_PIN;
    NTC_ANa = ANALOG_PIN;
    FL_ANa = ANALOG_PIN;
    VIN_ANt = INPUT_PIN;
    NTC_ANt = INPUT_PIN;
    FL_ANt = INPUT_PIN;

    AD1CON1 = 0x04E4; // 12-bit, integer, auto-sample and auto-conversion
    AD1CON2 = 0x0408; // IRQ after scanned 3 AN pins on CH0, VDD and VSS as reference
    AD1CON3 = 0x1F1F; // ADC clock derived from system clock, Auto-Sample Time bits: 31 Tad, ADC1 Conversion Clock Select bits: Tad = 32*Tp
    AD1CON4 = 0x0000; // DMA not used, results stored in ADC1BUF0-F
    AD1CSSH = 0x0000; // no scan channels select from here
	AD1CSSL = 0x0023; // Select Scan Pins: AN0, AN1, AN5
    AD1CHS0 = 0x0000; // VREF- for CH0 neg input, CH0SA bits ignored for CH0 pos input selection

    AD1CON1bits.ADON = 1; // enable ADC module
    delay_us(20); // ADC stabilization delay
}

u16 analog_read_fuellevel(void)
{
    return adc_val_fl;
}

u16 analog_read_watertemp(void)
{
    return adc_val_ntc;
}

u16 analog_read_inputvoltage(void)
{
    float voltage = adc_val_vin * 3.3/4095.0;
    float vdivider = voltage * (22.0+1.5)/1.5;
    return vdivider * 10.0; // 0.1 V resolution
}
