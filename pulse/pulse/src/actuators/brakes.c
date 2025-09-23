#include "pico/stdlib.h"
#include <stdio.h>
#include "../../include/default_pins.h"
#include "../../include/actuators/brakes.h"

void brakes_configure_pins()
{
    gpio_init(BRAKE_0_DIGITAL);
    gpio_set_dir(BRAKE_0_DIGITAL, GPIO_OUT);

    gpio_init(BRAKE_1_DIGITAL);
    gpio_set_dir(BRAKE_1_DIGITAL, GPIO_OUT);

    gpio_init(BRAKE_2_DIGITAL);
    gpio_set_dir(BRAKE_2_DIGITAL, GPIO_OUT);

    gpio_init(BRAKE_3_DIGITAL);
    gpio_set_dir(BRAKE_3_DIGITAL, GPIO_OUT);
}

void set_brakes(uint brake_pin, enum gpio_dir pin_conf)
{
    gpio_put(brake_pin, pin_conf);
}
