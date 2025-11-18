/*
#ifndef ENCODER_READ_H
#define ENCODER_READ_H

#include <stdint.h>

/* --- donanıma göre ayarla --- */
#define PIN_A       2       /* Kanal-A girişi */
#define PIN_B       3       /* Kanal-B girişi */
#define PIN_Z       4       /* İndeks (Z) girişi, opsiyonel */
#define ENCODER_SM  0       /* Kullanılan state machine numarası */

/* ---------- API ---------- */
#ifdef __cplusplus
extern "C" {
#endif

/* PIO-programı yükler, GPIO ve IRQ kurar. 0 döndürür. */
int      encoder_setup(void);

/* Güncel pozisyonu (pulse sayacı) döndürür. */
int32_t  encoder_get_position(void);

#ifdef __cplusplus
}
#endif

#endif
*/
// Core/Inc/encoder_driver.h

#ifndef INC_ENCODER_DRIVER_H_
#define INC_ENCODER_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

// Encoder (TIM) Handle'ını parametre olarak alır
void ENCODER_Init(TIM_HandleTypeDef *htim);

// Pozisyonu okur ve shared_data'ya yazar
void ENCODER_Update(TIM_HandleTypeDef *htim);

#ifdef __cplusplus
}
#endif

#endif /* INC_ENCODER_DRIVER_H_ */


/* HAL TIM Encoder Modu: PIO'nun yerine, STM32'nin donanımsal Timer Encoder
Modu kullanılmalıdır. Bu, CPU yükü olmadan pozisyon sayımı
için en verimli yöntemdir.*/
