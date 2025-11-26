// Elektromanyetik veya mekanik fren sistemlerini kontrol eden sürücüleri içerir
// brakes.h - GÜVENLİK GÜNCELLEMESİ
#ifndef BRAKES_H
#define BRAKES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "shared_data.h"

// Brake GPIO Tanımları
// bunlara gerek yok SİLLLL


// Fren Durumları
typedef enum {
    BRAKE_RELEASED = 0, // Frenler açık (Serbest)
    BRAKE_ENGAGED  = 1, // Frenler kilitli (Sıkılmış)
    BRAKE_ERROR    = 2
} BrakeState_t;

// Fren Sistemi Hata Kodları
typedef enum {
    BRAKE_ERROR_NONE = 0,
    BRAKE_ERROR_OVERCURRENT = 1,
    BRAKE_ERROR_SENSOR_FAIL = 2,
    BRAKE_ERROR_TIMEOUT = 3
} BrakeError_t;

BrakeError_t BRAKES_GetLastError(void);
void BRAKES_Init(void);
// yeni ekledim 
void BRAKES_SetState(BrakeState_t state);
/* Fren sistemini istenen duruma getirir.
 state: BRAKE_RELEASED veya BRAKE_ENGAGED */
void BRAKES_EmergencyEngage(void);
// Acil durum frenlemesi. Tüm frenleri maksimum güçte kilitler.
 
bool BRAKES_IsEngaged(void);

#ifdef __cplusplus
}
#endif

#endif


/*#ifndef BRAKES_H
#define BRAKES_H

#include "pico/stdlib.h"
#include "stdio.h"
#include "../default_pins.h"

void brakes_configure_pins(); //fren erisimi icin gpio pinlerini hazirlar

void set_brakes(uint brake_pin, enum gpio_dir pin_conf); //fren ciktilarini ayarlar. ornegin gpio x  1 versin vs.

#endif
*/
