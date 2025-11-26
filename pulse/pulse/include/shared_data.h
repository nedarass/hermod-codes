/*
 * Core/Inc/shared_data.h
 *
 * Açıklama: Sistemdeki tüm sensör ve aktüatör verilerini
 * hiyerarşik (katmanlı) bir yapıda tutan merkezi veri havuzudur.
 */

#ifndef INC_SHARED_DATA_H_
#define INC_SHARED_DATA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include <stdbool.h> // bool tipi için gerekli

// ============================================================================
// --- ALT YAPILAR (Sub-Structs) ---
// ============================================================================

// 1. ENCODER VERİLERİ (sensors/encoder.c için)
typedef struct {
    float velocity_mps;      // Anlık Hız (m/s)
    float position_m;        // Konum (m)
    int64_t total_pulse;     // Toplam Pulse Sayısı (Debug için)
} EncoderData_t;

// 2. VESC MOTOR SÜRÜCÜ VERİLERİ (actuators/vesc6.c için)
typedef struct {
    int32_t rpm;             // Motor Devri
    int32_t current;         // Motor Akımı (mA veya Amper/100)
    float voltage;           // Giriş Voltajı
    float temperature;       // Motor/Sürücü Sıcaklığı
} VESC_Data_t;

// 3. MPU9250 IMU SENSÖR VERİLERİ (sensors/mpu9250.c için)
typedef struct {
    float accel_x_mss;       // X Ekseni İvme (m/s^2)
    float accel_y_mss;       // Y Ekseni İvme
    float accel_z_mss;       // Z Ekseni İvme
    float gyro_x_dps;        // X Ekseni Açısal Hız (derece/saniye)
    float gyro_y_dps;        // Y Ekseni Açısal Hız
    float gyro_z_dps;        // Z Ekseni Açısal Hız
    float temp_c;            // Sensör Sıcaklığı (C)
} MPUSensor_t;

// ============================================================================
// --- ANA VERİ YAPISI ---
// ============================================================================

typedef struct {
    
    // --- A. SİSTEM GENEL DURUMU (System) ---
    struct {
        bool emergency_mode;     // ACİL DURUM MODU (True: Sistem Kilitli)
        uint32_t error_flags;    // Genel Hata Bayrakları
        uint32_t run_time_ms;    // Sistem çalışma süresi
    } system;

    // --- B. SENSÖRLER (Sensors) ---
    struct {
        EncoderData_t encoder;   // Encoder verileri burada
        MPUSensor_t mpu;         // MPU9250 verileri
        // İlerde eklenecekler:
        // BatterySensor_t battery;
    } sensors;

    // --- C. AKTÜATÖRLER (Actuators) ---
    struct {
        // 1. Blinker (Işık)
        uint8_t blinker_state;      // 0:Off, 1:On, 2:Blink
        
        // 2. Frenler (Brakes)
        uint8_t brake_state;        // 0:Released, 1:Engaged
        uint8_t brake_error;        // Hata Kodu
        uint32_t brake_engagement_time; // Son işlem zamanı
        
        // 3. Güç Kesici (Power Cut)
        uint8_t power_state;        // 0:Off, 1:On, 2:Emergency
        
        // 4. Ana Motor (Inverter / VESC)
        int32_t target_rpm;         // Hedeflenen Hız
        uint8_t inverter_state;     // Sürücü Durumu (Ready, Running, Error)
        VESC_Data_t vesc_status;    // VESC'den okunan gerçek veriler
        
    } actuators;

} shared_data_t;

// ============================================================================
// --- GLOBAL ERİŞİM ---
// ============================================================================

// Tüm .c dosyaları bu değişkeni kullanacak.
// shared_data.c dosyasında "shared_data_t shared_data;" olarak tanımlanmalı.
extern shared_data_t shared_data;


// ============================================================================
// --- HATA BAYRAKLARI (Bitmask) ---
// ============================================================================
#define ERR_FLAG_MPU_FAIL       (1 << 0)
#define ERR_FLAG_NTC_OOR        (1 << 1)
#define ERR_FLAG_COMM_TIMEOUT   (1 << 2)
#define ERR_FLAG_POWER_TRIP     (1 << 3)

#ifdef __cplusplus
}
#endif

#endif /* INC_SHARED_DATA_H_ */
