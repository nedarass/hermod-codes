// pulse/pulse/include/actuators/vesc6.h
// Eğer motorlar VESC (Vedder Electronic Speed Controller) tarafından sürülüyorsa,
// bu dosya VESC ile iletişim kurmak için özel protokolü içerir.
// vesc6.h 

#ifndef VESC6_H
#define VESC6_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "shared_data.h" // VESC_Data_t buradan geliyor
#include <stdint.h>
#include <stdbool.h>

// VESC UART Paket Yapısı için yardımcı tanımlar
#define VESC_PACKET_START 0x02
#define VESC_PACKET_END   0x03
#define VESC_TIMEOUT_MS   50

// --- FONKSİYON PROTOTİPLERİ ---

/* VESC sürücüsünü başlatır.
 huart: VESC'in bağlı olduğu UART (örn: &huart3) */
void VESC_Init(UART_HandleTypeDef *huart);

/* Hedef RPM değerini ayarlar. 
 rpm: İstenen devir sayısı */
bool VESC_SetRPM(int32_t rpm);

/* Hedef akım (Tork) değerini ayarlar.
 current_ma: Miliamper cinsinden akım (örn: 5000 = 5A) */
bool VESC_SetCurrent(int32_t current_ma);

/* VESC'den durum verisi (Telemetry) ister (GET_VALUES komutu gönderir). */
bool VESC_RequestStatus(void);

/* Gelen ham veri paketini ayrıştırır ve Shared Data'ya yazar.
 buffer: Gelen veri dizisi
 len: Veri uzunluğu */
bool VESC_ParseStatus(uint8_t *buffer, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif
