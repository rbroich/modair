#ifndef RELAY_H
#define RELAY_H

#include "common.h"

#define LED1t                       TRISBbits.TRISB15
#define LED1                        LATBbits.LATB15

#define RELAY1t                     TRISAbits.TRISA3
#define RELAY1                      LATAbits.LATA3

#define OPENDRAIN1t                 TRISBbits.TRISB13
#define OPENDRAIN1                  LATBbits.LATB13
#define OPENDRAIN2t                 TRISAbits.TRISA4
#define OPENDRAIN2                  LATAbits.LATA4

void iopins_init(void);
void led_toggle(void);
void iopins_sendrelay(u8 idx);
void iopins_sendod1(u8 idx);
void iopins_sendod2(u8 idx);
void* iopins_relay_menu(u8 idx, u8 key_input);
void* iopins_od1_menu(u8 idx, u8 key_input);
void* iopins_od2_menu(u8 idx, u8 key_input);
void iopins_printmenu(u8 idx, u8 state);

#endif
