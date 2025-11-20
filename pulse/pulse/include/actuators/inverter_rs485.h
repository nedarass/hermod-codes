// Podun ana motor sürücüsü (inverter) ile RS485 protokolü üzerinden iletişim kurarak motorun hızını veya torkunu kontrol eder
#ifndef INVERTER_RS485_H
#define INVERTER_RS485_H

#include "pico/stdlib.h"
#include "stdio.h"
#include "../default_pins.h"

void rs485_modbus_configure(); //modbus iletisimi icin konfigurasyon

// rs485 modbus üzerinden veri gönderme fonksiyonu
void rs485_write(uint8_t *message, uint8_t len);

#endif
