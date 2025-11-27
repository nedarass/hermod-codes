// pulse/pulse/include/actuators/brakes.h
// Elektromanyetik veya mekanik fren sistemlerini kontrol eden sürücüleri içerir
// brakes.h - GÜVENLİK GÜNCELLEMESİ

#ifndef BRAKES_H
#define BRAKES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "shared_data.h"
#include <stdbool.h> // bool tipi için eklendi

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

// --- FONKSİYON PROTOTİPLERİ ---

void BRAKES_Init(void);

/* Fren sistemini istenen duruma getirir.
   state: BRAKE_RELEASED veya BRAKE_ENGAGED */
void BRAKES_SetState(BrakeState_t state);

// Acil durum frenlemesi. Tüm frenleri maksimum güçte kilitler.
void BRAKES_EmergencyEngage(void);

bool BRAKES_IsEngaged(void);

BrakeError_t BRAKES_GetLastError(void);

#ifdef __cplusplus
}
#endif

#endif
