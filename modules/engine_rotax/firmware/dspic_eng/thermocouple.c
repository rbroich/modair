#include "thermocouple.h"
#include "params.h"
#include "ecan_mod.h"
#include "modair_bus.h"

extern const s_param_settings PARAM_LIST[PARAM_CNT];
//u8 thermocouple_status = 0;

void thermocouple_cntdwn(u8 idx)
{
    // Read value from either TC0 or TC1 (based on idx)
    u16 tmp_val = thermocouple_read(idx&0x1);
    // Bit D14-D3 is the 12-bit temperature reading in 0.25 degree C steps
    // Bit D2 is normally low, and goes high if the thermocouple input is open / disconnected (functinality expects T- to be grounded)
    // Bit D15 and D1 should always be low
    // Bit D0 is high impedance
    if ((tmp_val&0x04)||(tmp_val&0x02)||(tmp_val&0x8000)) {
        //thermocouple_status |= (1<<(idx&0x1));
        return;
    }
    tmp_val = tmp_val >> 3; // get rid of the lower 3 bits
    float tmp_f = (float)tmp_val / 4.0; // convert to degree's C
    ecan_tx_float(PARAM_LIST[idx].pid, MT_BROADCAST_VALUE, tmp_f); // Send value
}

void* thermocouple_fnc_homescreen(u8 idx, u8 key_input)
{
    // first process key input
    switch(key_input) {
        case TK_ROT_PUSH: return 0;
    }
    // then print updated console text
    int i;
    char rtxt[16*4];
    for (i=0;i<16*4;i++) rtxt[i] = ' ';
    strcopy(&rtxt[1+0*16], "Raw Values:");
    u16 tmp_val = thermocouple_read(idx&0x1);
    rtxt[2+1*16] = '0';
    rtxt[3+1*16] = 'x';
    mprint_int(&rtxt[4+1*16], tmp_val, 16, 4);
    tmp_val = tmp_val >> 3; // get rid of the lower 3 bits
    float tmp_f = (float)tmp_val / 4.0; // convert to degree's C
    mprint_float(&rtxt[2+2*16], tmp_f, 0, 2);
    ecan_tx_console(PARAM_LIST[idx].pid, rtxt);
    return &thermocouple_fnc_homescreen;
}

void thermocouple_init(void)
{
    TC_nCS1 = 1;
    TC_nCS2 = 1;
    TC_SCK = 0; // RP43
    TC_SCKt = OUTPUT_PIN;
    TC_SDIt = INPUT_PIN; // RP42
    TC_nCS1t = OUTPUT_PIN;
    TC_nCS2t = OUTPUT_PIN;

    _RP43R = 9; // TC_SCK (output) RB11 == RP43 <- SCK2 (TABLE 11-3 in DS70000657H)
    _SDI2R = 42; // TC_SDI (input)  RB10 == RP42 -> SDI2 (TABLE 11-2 in DS70000657H)

    SPI2CON1 = 0b0000110000111101; // disable SDO and SS, 16bit, master, P 1:16 and S 1:1 = 3.75 MHz mode
    SPI2STAT = 0b1000000000000000; // SPI Enable
}

u16 thermocouple_read(u8 tc_index)
{
    // Conversion time: min 0.22 seconds each (triggered by CS going high)
    if (tc_index==0)
        TC_nCS1 = 0;
    else TC_nCS2 = 0; // CS going low stops any active conversion

    delay_ns(200);
    SPI2BUF = 0;
    delay_ns(200);
    SPI2STATbits.SPIROV = 0;
    while (SPI2STATbits.SPIRBF == 0); // wait until receive complete
    u16 tmp = SPI2BUF;

    if (tc_index==0)
        TC_nCS1 = 1;
    else TC_nCS2 = 1; // trigger next conversion

    return tmp;
}