#include "pico/stdlib.h"
#include <stdio.h>
#include "../../include/default_pins.h"
#include "../../include/actuators/power_cut.h"

void power_cut_configure_pin()
{
    gpio_init(POWER_CUT_RELAY_DIGITAL);
    gpio_set_dir(POWER_CUT_RELAY_DIGITAL, GPIO_OUT);
}

void set_power_cut_relay(bool status)
{
    gpio_put(POWER_CUT_RELAY_DIGITAL, status);
}
