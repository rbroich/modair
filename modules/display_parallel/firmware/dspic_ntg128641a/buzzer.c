#include "buzzer.h"

void buzzer_init(void)
{
    BUZZa = DIGITAL_PIN;
    BUZZt = OUTPUT_PIN;
    BUZZ = 0;
}

void buzzer_toggle(void)
{
    BUZZ = !BUZZ;
}
