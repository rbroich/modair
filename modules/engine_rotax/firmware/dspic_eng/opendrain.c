#include "opendrain.h"

void opendrain_init(void)
{
    OPENDRAIN1t = OUTPUT_PIN;
    OPENDRAIN1 = 0;
    OPENDRAIN2t = OUTPUT_PIN;
    OPENDRAIN2 = 0;
}

void opendrain1_toggle(void)
{
    OPENDRAIN1 = !OPENDRAIN1;
}

void opendrain2_toggle(void)
{
    OPENDRAIN2 = !OPENDRAIN2;
}
