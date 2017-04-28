#include "enginehours.h"

u8 enginehours_flash_erased = 0;

void enginehours_tmr_irq(void)
{
    // if RPM > 0, start counting time
    // if RPM == 0, stop counting time, but don't write new value to FLASH yet
    // if time>120 seconds and flag set,
    //   erase FLASH page and set flag
    //   re-program everything other than Engine Hours and Maintainance Timer in the same page
    //   (erase takes 23.1 ms, so we need to do this before vin is lost)
}

void enginehours_vinlost_irq(void)
{
    // Bus power has been lost;
    // onboard capacitors will only supply the CPU with power for another ~10ms

    if (enginehours_flash_erased) {
        // write Engine Hours and Maintainance Timer values to FLASH memory
        // writing a single word takes 54 us
    }
}

void enginehours_init(void)
{
    enginehours_flash_erased = 0;
    // read Engine Hours and Maintainance Timer values from FLASH memory
}
