/*#include "../../include/control/control.h"
#include "../../include/control/communication.h"
#include <stdio.h>
#include "pico/stdlib.h"

int get_message_type_non_sdk(int8_t * data, int size)
{
}
*/
// Core/Src/control.c

#include "control.h"
#include "shared_data.h"
#include "communication.h"
#include "ntc_sensor.h" // C arayüzü ile C++ ntc_sensor.cpp'ye bağlanır
#include <stdio.h>
#include <string.h>
#include <math.h>

// main.c'de tanımlanan donanım handle'ları
extern ADC_HandleTypeDef hadc1;

// Global sistem durum yapısı
extern System_State_t g_system_state;

// Zamanlayıcılar (HAL_GetTick() milisaniye cinsinden kullanılır)
static uint32_t last_telemetry_tx = 0;
static uint32_t last_health_tx = 0;
static uint32_t last_sensor_update = 0;

void CONTROL_Init(void)
{
    // 1. Sistem Durum Yapısı Varsayılan Başlatma
    // shared_data.c'de memset yapıldı, burada ilk değerler verilir.
    g_system_state.velocity_mps = 0.0f;
    g_system_state.raw_voltage_mv = 48500;
    g_system_state.raw_current_ma = -2000;
    g_system_state.power_line_status = 1;
    g_system_state.cpu_temp_c = 35.0f;

    // 2. Sensör/Sürücü Modüllerini Başlat
    NTC_Init();
    // Diğer init fonksiyonları (Encoder, MPU, vb.) buraya eklenmeli

    // 3. Haberleşmeyi Başlat
    COMM_Init();
}

void CONTROL_Loop(void)
{
    uint32_t current_tick = HAL_GetTick();

    // 1. SENSÖR GÜNCELLEMESİ (Hızlı Döngü: 10ms)
    if (current_tick - last_sensor_update >= 10)
    {
        // Gerçek Sensör Okumaları
        NTC_Update(&hadc1); // ADC Handle'ı gönderilir
        // ENCODER_Update();
        // MPU9250_Update();

        // Simülasyon Veri Güncelleme
        g_system_state.velocity_mps += 0.005f;
        // Simülasyon: Hızlanma 10 m/s'de dursun.
        if (g_system_state.velocity_mps > 10.0f) g_system_state.velocity_mps = 10.0f;

        g_system_state.position_m = g_system_state.velocity_mps * (current_tick / 1000.0f);
        g_system_state.power_w = (float)g_system_state.raw_voltage_mv * (float)g_system_state.raw_current_ma / 1000000.0f;

        // Hata Kontrolü
        if (g_system_state.raw_voltage_mv < 40000) {
             g_system_state.error_flags |= ERR_FLAG_POWER_TRIP;
        } else {
             g_system_state.error_flags &= ~ERR_FLAG_POWER_TRIP;
        }

        last_sensor_update = current_tick;
    }


    // 2. TELEMETRİ GÖNDERİMİ (Her 100ms'de bir - Monitoring Page)
    if (current_tick - last_telemetry_tx >= 100)
    {
        COMM_SendTelemetryData();
        last_telemetry_tx = current_tick;
    }

    // 3. HEALTH CHECK GÖNDERİMİ (Her 1000ms'de bir - HermodPulse Page)
    if (current_tick - last_health_tx >= 1000)
    {
        COMM_SendHealthCheck();
        last_health_tx = current_tick;
    }
}

/* // Core/Src/control.c

// ... (Gerekli include'lar: shared_data.h, communication.h, ntc_sensor.h, mpu9250_driver.h, encoder_driver.h, navigation.h)

// main.c'de tanımlanan donanım handle'ları
extern ADC_HandleTypeDef hadc1;
extern I2C_HandleTypeDef hi2c1; // MPU için I2C Handle'ı (CubeMX'te hi2c1 olarak ayarlandığı varsayılır)
extern TIM_HandleTypeDef htim3; // Encoder için Timer Handle'ı (CubeMX'te htim3 olarak ayarlandığı varsayılır)

// Global sistem durum yapısı
extern System_State_t g_system_state;

// Zamanlayıcılar
static uint32_t last_telemetry_tx = 0;
static uint32_t last_health_tx = 0;
static uint32_t last_sensor_update = 0;
static const float DT = 0.01f; // Kontrol/Sensor Döngü Süresi (10ms)

void CONTROL_Init(void)
{
    // ... Varsayılan değerler atanır ...

    // 2. Sensör/Sürücü Modüllerini Başlat
    NTC_Init();
    MPU9250_Init(&hi2c1); // I2C Handle'ı gönderilir
    ENCODER_Init(&htim3);  // TIM Handle'ı gönderilir
    NAVIGATION_Init();

    // 3. Haberleşmeyi Başlat
    COMM_Init();

    printf("Sistem kontrol hazir.\r\n");
}

void CONTROL_Loop(void)
{
    uint32_t current_tick = HAL_GetTick();

    // 1. SENSÖR GÜNCELLEMESİ (Hızlı Döngü: 10ms)
    if (current_tick - last_sensor_update >= 10)
    {
        // --- 1. Ham Veri Okuma (Shared Data'ya yazar) ---
        NTC_Update(&hadc1);
        MPU9250_Update(&hi2c1); // I2C'den okur
        ENCODER_Update(&htim3); // Timer'dan okur

        // --- 2. Sensor Füzyonu (Hız ve Konumu Günceller) ---
        NAVIGATION_Update(DT); // 10ms (DT) ile Kalman Filtresi güncellemesi yapılır

        // Simülasyon Veri Güncelleme
        // ... (Simülasyon lojiği) ...

        last_sensor_update = current_tick;
    }


    // 2. TELEMETRİ GÖNDERİMİ (Her 100ms'de bir)
    if (current_tick - last_telemetry_tx >= 100)
    {
        COMM_SendTelemetryData();
        last_telemetry_tx = current_tick;
    }

    // 3. HEALTH CHECK GÖNDERİMİ (Her 1000ms'de bir)
    if (current_tick - last_health_tx >= 1000)
    {
        COMM_SendHealthCheck();
        last_health_tx = current_tick;
    }
}*/
