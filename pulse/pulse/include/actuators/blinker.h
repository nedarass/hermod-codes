#ifndef BLINKER_H
#define BLINKER_H

#include "pico/stdlib.h"
#include "stdio.h"
#include "../default_pins.h"

void blinker_configure_pin(); //fren erisimi icin gpio pinlerini hazirlar

void set_blinker(bool status); //fren ciktilarini ayarlar. ornegin gpio x  1 versin vs.

#endif
