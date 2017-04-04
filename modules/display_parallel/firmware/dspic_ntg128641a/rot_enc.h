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

#define rot_inc_flag       0b00000001
#define rot_dec_flag       0b00000010
#define rot_push_flag      0b00000100
#define rot_longpush_flag  0b00001000
#define rot_shift_inc_flag 0b00010000
#define rot_shift_dec_flag 0b00100000
#define rot_mod_flag       0b10000000

void rot_enc_irq(void);
void rot_enc_tmr(void);
void rot_enc_init(void);

#endif
