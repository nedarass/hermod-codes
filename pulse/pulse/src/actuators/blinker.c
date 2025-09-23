#include "pico/stdlib.h"
#include <stdio.h>
#include "../../include/default_pins.h"
#include "../../include/actuators/blinker.h"

void blinker_configure_pin()
{
    gpio_init(BLINKER_DIGITAL);
    gpio_set_dir(BLINKER_DIGITAL, GPIO_OUT);
}

void set_blinker(bool status)
{
    gpio_put(BLINKER_DIGITAL, status);
}
