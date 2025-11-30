// pulse/pulse/include/actuators/cooling.h
// Batarya ve Motor soğutma fanlarının kontrolü
#ifndef COOLING_H
#define COOLING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdbool.h>

// Başlatma
void COOLING_Init(void);

/* Fanları AÇAR veya KAPATIR.
   Donanım Notu: 4 Fan seri bağlı olduğu için hepsi aynı anda açılır/kapanır.
   Hız kontrolü (PWM) bu konfigürasyonda mümkün değildir. 
   state: true (AÇIK), false (KAPALI) */
void COOLING_SetFanState(bool state);

#ifdef __cplusplus
}
#endif
#endif

