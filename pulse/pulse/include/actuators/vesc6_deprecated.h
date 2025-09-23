#ifndef VESC6_H_
#define VESC6_H_
#include "pico/stdlib.h"
#include "stdio.h"
#include "../default_pins.h"
#include "hardware/pwm.h"

extern volatile uint slice1, slice2, slice3, slice4;
extern volatile uint chan1, chan2, chan3, chan4;

void vesc6_configure();
void set_motor_duty_cycle(uint slice, uint channel, uint16_t level);

#endif
