
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
typedef enum {
    BLINKER_STATE_OFF = 0,
    BLINKER_STATE_ON = 1,
    BLINKER_STATE_BLINKING = 2
} BlinkerState_t;

void BLINKER_Init(void);
void BLINKER_Set(bool status); 
// Belirli bir süre boyunca LED'i yakıp söndürür (Non-blocking olması tercih edilir). Control döngüsü içinde sayaç ile yönetilmesi daha uygundur.
void BLINKER_Toggle(void);
void BLINKER_UpdateNonBlocking(void); // 100ms timer interrupt'ta çağrılacak // Non-blocking yanıp sönme için (Timer interrupt veya loop içinde çağrılır)
void BLINKER_StartBlinking(void);
void BLINKER_StopBlinking(void) ;
#ifdef __cplusplus
}
#endif

#endif

