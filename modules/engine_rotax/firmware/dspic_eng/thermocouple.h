#ifndef THERMOCOUPLE_H
#define THERMOCOUPLE_H

#include "common.h"

#define TC_SCKt                   TRISBbits.TRISB11
#define TC_SCK                    LATBbits.LATB11
#define TC_SDIt                   TRISBbits.TRISB10
#define TC_SDI                    PORTBbits.RB10
#define TC_nCS2t                  TRISBbits.TRISB6
#define TC_nCS2                   LATBbits.LATB6
#define TC_nCS1t                  TRISBbits.TRISB5
#define TC_nCS1                   LATBbits.LATB5


void thermocouple_init(void);
u16 thermocouple_read(u8 tc_index);

#endif
