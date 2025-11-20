// Eğer motorlar VESC (Vedder Electronic Speed Controller) tarafından sürülüyorsa,
// bu dosya VESC ile iletişim kurmak için özel protokolü (genellikle UART veya CAN) içerir
// vesc6.h - GELİŞMİŞ PROTOCOL
#ifndef VESC6_H
#define VESC6_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "shared_data.h"
#include <stdint.h>

// VESC UART Paket Yapısı için yardımcı tanımlar
#define VESC_PACKET_START 0x02
#define VESC_PACKET_END   0x03
#define VESC_TIMEOUT_MS   50

typedef struct {
    int32_t rpm;
    int32_t current;
    float voltage;
    float temperature;
    uint8_t error_flags;
} VESC_Status_t;

void VESC_Init(UART_HandleTypeDef *huart);
/* VESC sürücüsünü başlatır.
 huart: VESC'in bağlı olduğu UART (örn: &huart3)
 */
bool VESC_SetRPM(int32_t rpm);
/* Hedef RPM değerini ayarlar. VESC protokolüne uygun veri paketini hazırlar ve gönderir.
 rpm: İstenen devir sayısı
 */
bool VESC_SetCurrent(int32_t current_ma);
/* Hedef akım (Tork) değerini ayarlar.
 current_ma: Miliamper cinsinden akım (örn: 5000 = 5A)
 */
bool VESC_RequestStatus(void);
bool VESC_ParseStatus(VESC_Status_t *status);
uint16_t VESC_CalculateChecksum(uint8_t *data, uint16_t len);
/* VESC protokolü için CRC16 checksum hesaplar.
 .c dosyasında implemente edilecektir.
 */

#ifdef __cplusplus
}
#endif

#endif















/*#ifndef VESC6_H
#define VESC6_H
#include "pico/stdlib.h"
#include "stdio.h"
#include "../default_pins.h"
#include "hardware/pwm.h"

void vesc6_configure(); //connect pico <-> vesc 6
uint8_t calculate_checksum(uint8_t *data, int len); //veri butunlugunu dogrulamak icin checksum hazirlilyor
void set_motor_rpm(int rpm); //motor donus hizlarini ayarliyor

#endif
*/
