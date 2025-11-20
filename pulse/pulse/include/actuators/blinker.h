
//Podun dışarıya (veya içerideki ekiplere) durum bilgisi vermek için kullanılan ışıkların kontrolü
// blinker.h - DÜZENLENMİŞ HALİ
#ifndef BLINKER_H
#define BLINKER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdbool.h>
#include "shared_data.h"
/*Durum LED'ini yakar veya söndürür.
 Not: Pin konfigürasyonu CubeMX (main.c) tarafından yapılmıştır.
 status: true (LED AÇIK), false (LED KAPALI)
 */
// GPIO Tanımları - CubeMX'te yapılandırılacak
#define BLINKER_GPIO_PORT GPIOA
#define BLINKER_PIN       GPIO_PIN_5

typedef enum {
    BLINKER_STATE_OFF = 0,
    BLINKER_STATE_ON = 1,
    BLINKER_STATE_BLINKING = 2
} BlinkerState_t;

void BLINKER_Init(void);
void BLINKER_Set(bool status); 
/* Belirli bir süre boyunca LED'i yakıp söndürür (Non-blocking olması tercih edilir).
 Control döngüsü içinde sayaç ile yönetilmesi daha uygundur.
 */
void BLINKER_Toggle(void);
void BLINKER_UpdateNonBlocking(void); // 100ms timer interrupt'ta çağrılacak

#ifdef __cplusplus
}
#endif

#endif







/*#ifndef BLINKER_H
#define BLINKER_H

#include "pico/stdlib.h"
#include "stdio.h"
#include "../default_pins.h"

void blinker_configure_pin(); //fren erisimi icin gpio pinlerini hazirlar

void set_blinker(bool status); //fren ciktilarini ayarlar. ornegin gpio x  1 versin vs.

#endif
*/
