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
// mpu9250.c ile uyumlu olması için isimleri kısalttım:
#define ERR_NONE              0
#define ERR_ENCODER_INIT      (1 << 0) 
#define ERR_MPU_FAIL          (1 << 1) // Artık mpu9250.c hatası ile uyumlu
#define ERR_NTC_OOR           (1 << 2) 
#define ERR_COMM_TIMEOUT      (1 << 3) 
#define ERR_POWER_TRIP        (1 << 4) 
#define ERR_HEALTH_CRITICAL   (1 << 5) 

// ============================================================================
// --- ALT YAPILAR (Sub-Structs) ---
// ============================================================================

// 1. ENCODER VERİLERİ 
typedef struct {
    float velocity_mps;      // Anlık Hız (m/s)
    float position_m;        // Konum (m)
    int64_t total_pulse;     // Toplam Pulse Sayısı
} EncoderData_t;

// 2. VESC MOTOR SÜRÜCÜ VERİLERİ 
typedef struct {
    int32_t rpm;             
    int32_t current;         
    float voltage;           
    float temperature;       
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
    uint8_t obstacle_detected; 
    uint8_t raw_sensor_1;      
    uint8_t raw_sensor_2;      
} OpticsData_t;

// ============================================================================
// --- ANA VERİ YAPISI (Main Struct) ---
// ============================================================================

typedef struct {
    
    // --- A. SİSTEM GENEL DURUMU (System) ---
    struct {
        bool emergency_mode;     
        uint32_t error_flags;    
        uint32_t run_time_ms;    
        uint32_t health_status;  
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
