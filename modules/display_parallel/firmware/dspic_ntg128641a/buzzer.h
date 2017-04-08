#ifndef BUZZER_H
#define BUZZER_H

#include "common.h"

#define BUZZa                   ANSELAbits.ANSA1
#define BUZZt                   TRISAbits.TRISA1
#define BUZZ                    LATAbits.LATA1

void buzzer_init(void);
void buzzer_toggle(void);

#endif
