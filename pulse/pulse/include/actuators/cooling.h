// cooling.h - ÖNERİ
// Motor sürücüler (VESC/Inverter) ve Li-Po bataryalar kapalı bir kabuk (pod) içinde çok ısınır. Basit bir MOSFET veya Röle üzerinden kontrol edilen fanlara
#ifndef COOLING_H
#define COOLING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

// Fan seviyeleri (Eğer PWM kullanacaksan) veya sadece AÇ/KAPA
void COOLING_Init(void);
void COOLING_SetFanState(bool state); // Tüm fanları aç/kapa
void COOLING_SetFanSpeed(uint8_t speed_percent); // PWM varsa %0-100 arası

#ifdef __cplusplus
}
#endif
#endif
