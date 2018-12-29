#ifndef OPENDRAIN_H
#define OPENDRAIN_H

#include "common.h"

#define OPENDRAIN1t                   TRISBbits.TRISB13
#define OPENDRAIN1                    LATBbits.LATB13
#define OPENDRAIN2t                   TRISAbits.TRISA4
#define OPENDRAIN2                    LATAbits.LATA4

void opendrain_init(void);
void opendrain1_toggle(void);
void opendrain2_toggle(void);

#endif
