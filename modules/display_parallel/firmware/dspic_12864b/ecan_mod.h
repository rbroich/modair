#ifndef ECAN_MOD_H
#define ECAN_MOD_H

#include "common.h"

#define NR_ECAN_BUF             16 // first 8 are Tx, last 8 are Rx

#define U1TXt                   TRISBbits.TRISB7
#define U1TX                    LATBbits.LATB7
#define U1RXa                   ANSELBbits.ANSB8
#define U1RXt                   TRISBbits.TRISB8
#define U1RX                    PORTBbits.RB8

void ecan_irq(void);
u8 ecan_tx(u32 cid, u8 len, u8 rtr, u16 *data);
// extern void ecan_rx(u32 cid, u8 len, u8 rtr, u16 *data);
void ecan_init(void);

#endif
