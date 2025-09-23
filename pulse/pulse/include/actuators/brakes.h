#ifndef BRAKES_H
#define BRAKES_H

#include "pico/stdlib.h"
#include "stdio.h"
#include "../default_pins.h"

void brakes_configure_pins(); //fren erisimi icin gpio pinlerini hazirlar

void set_brakes(uint brake_pin, enum gpio_dir pin_conf); //fren ciktilarini ayarlar. ornegin gpio x  1 versin vs.

#endif
