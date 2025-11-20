// buzzer.h - ÖNERİ
//güvenlik için pod hareket etmeden önce sesli uyarı vermesi (Ready-to-Drive Sound) gerekebilir
#ifndef BUZZER_H
#define BUZZER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

void BUZZER_Init(void);
void BUZZER_Beep(uint16_t duration_ms); // Kısa bip
void BUZZER_Alarm(void); // Sürekli alarm modu (Acil durum)

#ifdef __cplusplus
}
#endif
#endif
