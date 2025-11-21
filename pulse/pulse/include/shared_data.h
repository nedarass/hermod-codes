/*
 * Core/Inc/shared_data.h
 *
 * Açıklama: Sistemdeki tüm sensör verilerini ve durum bayraklarını
 * tutan merkezi veri yapısıdır.
 */

#ifndef INC_SHARED_DATA_H_
#define INC_SHARED_DATA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h" // HAL tipleri ve uint tanımları için
#include <stdint.h>

// ---------------------------------------------------------------------------
// --- MERKEZİ SİSTEM DURUM YAPISI ---
// ---------------------------------------------------------------------------
typedef struct {
    // --- SENSÖR VE FİZİKSEL VERİLER (Monitoring Page) ---
    float velocity_mps;         // Anlık Hız (m/s) - [Encoder + Navigasyon]
    float acceleration_mss;     // Anlık İvme (m/s^2) - [MPU9250]
    float position_m;           // Konum (m) - [Encoder + Navigasyon]
    
    uint16_t raw_voltage_mv;    // Batarya Voltajı (mV)
    int16_t raw_current_ma;     // Çekilen Akım (mA)
    float power_w;              // Anlık Güç (W)
    
    int16_t raw_temp_ntc1;      // Sıcaklık (Santigrat * 100 formatında. Örn: 2550 = 25.50 C)
    
    uint8_t brake_status;       // Fren Durumu (0: Açık/Serbest, 1: Kapalı/Frenli)

    // --- SİSTEM SAĞLIK VERİLERİ (Health Check Page) ---
    float cpu_temp_c;           // İşlemci Sıcaklığı
    uint32_t error_flags;       // Hata Bayrakları (Bitmask)
    uint16_t ping_ms;           // Haberleşme Gecikmesi (ms)
    uint8_t power_line_status;  // Güç Hattı Durumu (0: OFF, 1: ON)
    uint8_t rtos_task_status;   // (Opsiyonel) Görev Durumu

} System_State_t;


// ---------------------------------------------------------------------------
// --- GLOBAL ERİŞİM ---
// ---------------------------------------------------------------------------
// Bu değişken shared_data.c'de oluşturulur, diğer herkes buradan erişir.
extern System_State_t g_system_state;


// ---------------------------------------------------------------------------
// --- HATA BAYRAKLARI (Bitmask Tanımları) ---
// ---------------------------------------------------------------------------
// error_flags değişkeni içindeki bitlerin anlamları
#define ERR_FLAG_MPU_FAIL       (1 << 0) // Bit 0: MPU Sensör Hatası
#define ERR_FLAG_NTC_OOR        (1 << 1) // Bit 1: Sıcaklık Limit Dışı (Out of Range)
#define ERR_FLAG_COMM_TIMEOUT   (1 << 2) // Bit 2: Haberleşme Kesildi
#define ERR_FLAG_POWER_TRIP     (1 << 3) // Bit 3: Güç Kesintisi/Düşük Voltaj

#ifdef __cplusplus
}
#endif

#endif /* INC_SHARED_DATA_H_ */
