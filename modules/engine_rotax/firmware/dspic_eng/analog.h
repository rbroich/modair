#ifndef ANALOG_H
#define ANALOG_H

#include "common.h"

#define VIN_ANa                  _ANSA0   // AN0
#define VIN_ANt                  _TRISA0
#define VIN_AN                   _RA0

#define NTC_ANa                  _ANSA1   // AN1
#define NTC_ANt                  _TRISA1
#define NTC_AN                   _RA1

#define FL_ANa                   _ANSB3   // AN5
#define FL_ANt                   _TRISB3
#define FL_AN                    _RB3

void analog_irq(void);
void analog_init(void);
u16 analog_read_fuellevel(void);
u16 analog_read_watertemp(void);
u16 analog_read_inputvoltage(void);

#endif
