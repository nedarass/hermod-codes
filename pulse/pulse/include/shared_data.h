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
#include <stdbool.h> 

// ============================================================================
// --- HATA BAYRAKLARI (Bitmask Macros) ---
// ============================================================================
// Bu bitler system.error_flags değişkeni içinde set edilecek.
#define ERR_NONE              0
#define ERR_ENCODER_INIT      (1 << 0) // 0000 0001
#define ERR_MPU_FAIL          (1 << 1) // 0000 0010
#define ERR_NTC_OOR           (1 << 2) // 0000 0100 (Out of Range)
#define ERR_COMM_TIMEOUT      (1 << 3) // 0000 1000
#define ERR_POWER_TRIP        (1 << 4) // 0001 0000
#define ERR_HEALTH_CRITICAL   (1 << 5) // Kritik Sağlık Sorunu

// ============================================================================
// --- ALT YAPILAR (Sub-Structs) ---
// ============================================================================

// 1. ENCODER VERİLERİ 
typedef struct {
    float velocity_mps;      // Anlık Hız (m/s)
    float position_m;        // Konum (m)
    int64_t total_pulse;     // Toplam Pulse Sayısı (Debug için)
} EncoderData_t;

// 2. VESC MOTOR SÜRÜCÜ VERİLERİ 
typedef struct {
    int32_t rpm;             // Motor Devri
    int32_t current;         // Motor Akımı 
    float voltage;           // Giriş Voltajı
    float temperature;       // Motor/Sürücü Sıcaklığı
} VESC_Data_t;

// 3. MPU9250 IMU SENSÖR VERİLERİ 
typedef struct {
    float accel_x_mss;       
    float accel_y_mss;       
    float accel_z_mss;       
    float gyro_x_dps;        
    float gyro_y_dps;        
    float gyro_z_dps;        
    float temp_c;            
} MPUSensor_t;

// 4. NAVİGASYON ÇIKTILARI 
typedef struct {
    float position_m;        
    float velocity_mps;      
    float accel_bias_mss;    
} NavigationData_t;

// 5. BATARYA VE SICAKLIK VERİLERİ 
typedef struct {
    uint16_t voltage_mv;     
    int16_t current_ma;      
    float ntc_temp_c;        
} BatteryData_t;

// 6. OPTİK SENSÖRLER 
typedef struct {
    uint8_t obstacle_detected; // 0: Yok, 1: VAR 
    uint8_t raw_sensor_1;      
    uint8_t raw_sensor_2;      
} OpticsData_t;

// ============================================================================
// --- ANA VERİ YAPISI (Main Struct) ---
// ============================================================================

typedef struct {
    
    // --- A. SİSTEM GENEL DURUMU (System) ---
    struct {
        bool emergency_mode;     // ACİL DURUM MODU (True: Sistem Kilitli)
        uint32_t error_flags;    // Hata Bayrakları (Yukarıdaki makrolar buraya yazılır)
        uint32_t run_time_ms;    // Sistem çalışma süresi
        uint32_t health_status;  // Ekstra sağlık durumu (Opsiyonel)
    } system;

    // --- B. SENSÖRLER (Sensors) ---
    struct {
        EncoderData_t encoder;   
        MPUSensor_t mpu;         
        NavigationData_t nav;
        BatteryData_t battery;
        OpticsData_t optics;
    } sensors;

    // --- C. AKTÜATÖRLER (Actuators) ---
    struct {
        // 1. Blinker (Işık)
        uint8_t blinker_state;   
        
        // 2. Frenler (Brakes)
        uint8_t brake_state;     
        uint8_t brake_error;     
        uint32_t brake_engagement_time; 
        
        // 3. Güç Kesici (Power Cut)
        uint8_t power_state;     
        
        // 4. Ana Motor (Inverter / VESC)
        int32_t target_rpm;      
        uint8_t inverter_state;  
        VESC_Data_t vesc_status; 
        
    } actuators;

} shared_data_t; 

// ============================================================================
// --- GLOBAL ERİŞİM ---
// ============================================================================

extern shared_data_t shared_data;

#ifdef __cplusplus
}
#endif

#endif /* INC_SHARED_DATA_H_ */
