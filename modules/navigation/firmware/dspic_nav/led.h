#ifndef LED_H
#define LED_H

#include "common.h"

#define LED1t                   TRISBbits.TRISB15
#define LED1                    LATBbits.LATB15

void led_init(void);
void led_toggle(void);

#endif
