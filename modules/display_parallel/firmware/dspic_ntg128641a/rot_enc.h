#ifndef ROT_ENC_H
#define ROT_ENC_H

#include "common.h"

#define PB0a                    ANSELBbits.ANSB1
#define PB0t                    TRISBbits.TRISB1
#define PB0c                    CNENBbits.CNIEB1
#define PB0pu                   CNPUBbits.CNPUB1
#define PB01                    PORTBbits.RB1

#define ROT0a                   ANSELBbits.ANSB2
#define ROT0t                   TRISBbits.TRISB2
#define ROT0c                   CNENBbits.CNIEB2
#define ROT0pu                  CNPUBbits.CNPUB2
#define ROT0                    PORTBbits.RB2

#define ROT1a                   ANSELBbits.ANSB3
#define ROT1t                   TRISBbits.TRISB3
#define ROT1c                   CNENBbits.CNIEB3
#define ROT1pu                  CNPUBbits.CNPUB3
#define ROT1                    PORTBbits.RB3

#define C_ROT_INC               0x81
#define C_ROT_DEC               0x82
#define C_ROT_HOLD_INC          0x83
#define C_ROT_HOLD_DEC          0x84
#define C_ROT_PUSH              0x85
#define C_ROT_HOLD              0x86
#define C_ROT_LONGHOLD          0x87
#define C_ROT_EXTRALONGHOLD     0x88

void rot_enc_irq(void);
void rot_enc_tmr(void);
void rot_enc_init(void);

#endif
