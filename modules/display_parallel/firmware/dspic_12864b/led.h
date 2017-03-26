#ifndef LED_H
#define LED_H

#include "common.h"

#define LED1a                   ANSELAbits.ANSA0
#define LED1t                   TRISAbits.TRISA0
#define LED1                    LATAbits.LATA0

void led_init(void);
void led_toggle(void);

#endif
