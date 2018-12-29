#include "relay.h"

void relay_init(void)
{
    RELAY1t = OUTPUT_PIN;
    RELAY1 = 0;
}

void relay_toggle(void)
{
    RELAY1 = !RELAY1;
}
