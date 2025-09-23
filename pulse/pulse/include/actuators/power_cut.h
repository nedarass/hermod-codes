#ifndef POWER_CUT_H
#define POWER_CUT_H

#include "pico/stdlib.h"
#include "stdio.h"
#include "../default_pins.h"

void power_cut_configure_pin(); //fren erisimi icin gpio pinlerini hazirlar

void set_power_cut_relay(bool status); //fren ciktilarini ayarlar. ornegin gpio x  1 versin vs.

#endif
