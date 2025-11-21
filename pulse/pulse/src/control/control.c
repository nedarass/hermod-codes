 /**
  ******************************************************************************
  * @file           : control.c
  * @brief          : Ana Kontrol Döngüsü ve Sistem Yönetimi
  ******************************************************************************
  */

#include "control.h"
#include "shared_data.h"
#include "communication.h"
#include "commands.h"

// --- SÜRÜCÜ BAŞLIK DOSYALARI ---
#include "ntc_sensor.h"
#include "encoder.h"
#include "mpu9250_driver.h"
#include "navigation.h"

#include <stdio.h> // printf için

// --- DONANIM HANDLE'LARI (Main.c'den gelir) ---
// Bu isimlerin CubeMX'teki tanımlarla birebir aynı olduğundan emin olun!
extern ADC_HandleTypeDef hadc1; // NTC Sensörü için
extern TIM_HandleTypeDef htim3; // Encoder için (Encoder Modunda)
extern I2C_HandleTypeDef hi2c1; // MPU9250 için

// --- GLOBAL SİSTEM DURUMU ---
extern System_State_t g_system_state;

// --- ZAMANLAYICI DEĞİŞKENLERİ ---
static uint32_t last_sensor_update = 0; // 10ms (100 Hz)
static uint32_t last_telemetry_tx = 0;  // 100ms (10 Hz)
static uint32_t last_health_tx = 0;     // 1000ms (1 Hz)

// Navigasyon ve Kontrol döngüsü periyodu (Saniye cinsinden)
static const float DT = 0.01f; 

// -------------------------------------------------------------------
// --- BAŞLATMA FONKSİYONU ---
// -------------------------------------------------------------------
void CONTROL_Init(void)
{
    // 1. Sensör Sürücülerini Başlat
    // Donanım adreslerini (Handle) sürücülere gönderiyoruz.
    
    NTC_Init(); 
    
    ENCODER_Init(&htim3); 
    
    // MPU9250 Başlatma (Başarısız olursa hata bayrağı dikilir)
    if (MPU9250_Init(&hi2c1) != HAL_OK) {
        g_system_state.error_flags |= ERR_FLAG_MPU_FAIL;
        printf("MPU9250 Baslatma Hatasi!\r\n");
    }

    // Navigasyon (Kalman Filtresi) Başlatma
    NAVIGATION_Init();

    // 2. Haberleşmeyi Başlat (UART Kesmesi)
    COMM_Init();
    
    // 3. Sistem Hazır Mesajı
    printf("CONTROL: Sistem baslatildi. Dongu basliyor.\r\n");
}

// ÇÖZÜM: Error handling ve recovery ekle
void CONTROL_HandleErrors(void)
{
    // MPU hatası
    if (g_system_state.error_flags & ERR_FLAG_MPU_FAIL) {
        static uint32_t mpu_retry_time = 0;
        if (HAL_GetTick() - mpu_retry_time > 5000) { // 5 saniyede bir retry
            if (MPU9250_Init(&hi2c1) == HAL_OK) {
                g_system_state.error_flags &= ~ERR_FLAG_MPU_FAIL;
                printf("MPU9250 recovery successful\r\n");
            }
            mpu_retry_time = HAL_GetTick();
        }
    }
    
    // Communication hatası
    if (g_system_state.error_flags & ERR_FLAG_COMM_TIMEOUT) {
        // Yeniden başlatmayı dene
        COMM_Init();
    }
}



// -------------------------------------------------------------------
// --- ANA KONTROL DÖNGÜSÜ ---
// -------------------------------------------------------------------
void CONTROL_Loop(void)
{
    uint32_t current_tick = HAL_GetTick();

    // ============================================================
    // 1. SENSÖR OKUMA VE NAVİGASYON (Her 10ms'de bir)
    // ============================================================
    if (current_tick - last_sensor_update >= 10)
    {
       // sonradan eklendi 
        CONTROL_HandleErrors();
        // --- A. Sensör Güncellemeleri ---
        
        // NTC (Sıcaklık) Okuması:
        // Polling modunda çalışır, işlemciyi çok kısa süre meşgul eder.
        NTC_Update(&hadc1); 

        // Encoder (Hız/Konum) Okuması:
        // Timer donanımından anlık değeri çeker. Çok hızlıdır.
        ENCODER_Update(&htim3);

        // MPU9250 (İvme/Jiro) Okuması (DMA Modu):
        // Sadece okumayı TETİKLER. Veri arka planda gelir ve kesme (IRQ) ile işlenir.
        // CPU'yu bekletmez.
        MPU9250_Trigger_Read(&hi2c1); 

        
        // --- B. Sensör Füzyonu (Navigation) ---
        // MPU ve Encoder'dan gelen en son verileri birleştirir.
        // Not: MPU verisi bir önceki döngüden veya DMA kesmesinden gelmiş olabilir.
        NAVIGATION_Update(DT); 

        
        // --- C. Güvenlik Kontrolleri ---
        // Örnek: Aşırı sıcaklık kontrolü
        if (g_system_state.raw_temp_ntc1 > 8500) { // 85.00 C
             // Aşırı sıcaklık durumunda yapılacaklar (örn: Güç azaltma)
        }

        last_sensor_update = current_tick;
    }

    // ============================================================
    // 2. TELEMETRİ GÖNDERİMİ (Her 100ms'de bir)
    // ============================================================
    if (current_tick - last_telemetry_tx >= 100)
    {
        // Hız, Konum, İvme, Sıcaklık vb. verileri Bifrost'a gönder
        COMM_SendTelemetryData();
        last_telemetry_tx = current_tick;
    }

    // ============================================================
    // 3. SAĞLIK KONTROLÜ (HEALTH CHECK) (Her 1000ms'de bir)
    // ============================================================
    if (current_tick - last_health_tx >= 1000)
    {
        // CPU Sıcaklığı, Hata Bayrakları, Ping Süresi vb. gönder
        COMM_SendHealthCheck();
        last_health_tx = current_tick;
    }
}
