#ifndef ECAN_MOD_H
#define ECAN_MOD_H

#include "common.h"

#define NR_ECAN_BUF             16 // first 8 are Tx, last 8 are Rx

#define U1TXt                   TRISBbits.TRISB7
#define U1TX                    LATBbits.LATB7
#define U1RXa                   ANSELBbits.ANSB2
#define U1RXt                   TRISBbits.TRISB2
#define U1RX                    PORTBbits.RB2

#define ECAN_FLAGS_RTR          0x04

void ecan_process(void);
void ecan_tx_console(u16 pid, char *str);
void ecan_tx_float(u16 pid, u16 msg_type, float val);
void ecan_tx_str(u16 pid, char* str, u16 msg_type, u8 len);
void ecan_tx(u16 pid, u16 d0, u16 d2, u16 d4, u16 d6, u16 msg_type, u8 len, u8 flags);
void ecan_init(void);

#endif
