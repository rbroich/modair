#include "led.h"

void led_init(void)
{
    LED1t = OUTPUT_PIN;
    LED1 = 0;
}

void led_toggle(void)
{
    LED1 = !LED1;
}
