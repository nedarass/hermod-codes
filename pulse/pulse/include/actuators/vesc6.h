// Eğer motorlar VESC (Vedder Electronic Speed Controller) tarafından sürülüyorsa, bu dosya VESC ile iletişim kurmak için özel protokolü (genellikle UART veya CAN) içerir
#ifndef VESC6_H
#define VESC6_H
#include "pico/stdlib.h"
#include "stdio.h"
#include "../default_pins.h"
#include "hardware/pwm.h"

void vesc6_configure(); //connect pico <-> vesc 6
uint8_t calculate_checksum(uint8_t *data, int len); //veri butunlugunu dogrulamak icin checksum hazirlilyor
void set_motor_rpm(int rpm); //motor donus hizlarini ayarliyor

#endif
