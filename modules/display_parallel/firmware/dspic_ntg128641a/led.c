#include "led.h"

void led_init(void)
{
    LED1a = DIGITAL_PIN;
    LED1t = OUTPUT_PIN;
    LED1 = 1;
}

void led_toggle(void)
{
    LED1 = !LED1;
}
