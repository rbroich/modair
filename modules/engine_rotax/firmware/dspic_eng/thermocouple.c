#include "thermocouple.h"

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
    // Conversion time: 0.22 seconds each (triggered by CS going high)

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
    else TC_nCS2 = 1; // trigger next conversion (0.22 sec)

    if (tmp & 0b100) // thermocouple open if bit set
        return 0;
    tmp = tmp >> 3;

    return tmp;
}