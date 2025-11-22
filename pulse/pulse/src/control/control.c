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

// --- AKTÜATÖR SÜRÜCÜLERİ (Otonom kontrol için) ---
//#include "actuators/brakes.h"      // Fren kontrolü
//#include "actuators/vesc6.h"       // Motor kontrolü
//#include "actuators/power_cut.h"   // Güç kesme kontrolü

#include <stdio.h> // printf için
#include <math.h>

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
void CONTROL_AutonomousDecisions(void)
{
    // Hyperloop için gerçekçi değerler (tahmini)
    const float MAX_SAFE_SPEED = 80.0f;        // m/s (~288 km/h)
    const float CRITICAL_TEMP = 75.0f;         // °C - Motor sıcaklığı
    const float WARNING_TEMP = 60.0f;          // °C  
    const float LOW_VOLTAGE_THRESHOLD = 44.0f; // V (12S Lipo ~44V min)
    const float CRITICAL_VOLTAGE = 42.0f;      // V
    
    float current_temp = g_system_state.raw_temp_ntc1 / 100.0f;
    float current_voltage = g_system_state.raw_voltage_mv / 1000.0f;

    // ========================
    // 🔴 KRİTİK ACİL DURUMLAR - SİSTEMİ TAMAMEN DURDUR
    // ========================
    
    // 1. ACİL: ÇOK YÜKSEK HIZ
    if (g_system_state.velocity_mps > EMERGENCY_SPEED) {
        printf("🚨🚨 ACİL DURUM: Asiri hiz %.1f m/s > %.1f m/s\r\n", 
               g_system_state.velocity_mps, EMERGENCY_SPEED);
        
        SYSTEM_EmergencyPowerCut();  // commands.c'den
        BRAKES_EmergencyEngage();    // brakes.h'den
        
        return; // Fonksiyondan çık, başka karar alınmasın
    }
    
    // 2. ACİL: KRİTİK VOLTAJ
    if (current_voltage < CRITICAL_VOLTAGE) {
        printf("🚨🚨 ACİL DURUM: Kritik voltaj %.1fV < %.1fV\r\n", 
               current_voltage, CRITICAL_VOLTAGE);
        
        SYSTEM_EmergencyPowerCut();
        BRAKES_EmergencyEngage();
        
        return;
    }
    
    // 3. ACİL: ÇOK YÜKSEK SICAKLIK + YÜKSEK HIZ
    if (current_temp > CRITICAL_TEMP + 10.0f && g_system_state.velocity_mps > 20.0f) {
        printf("🚨🚨 ACİL DURUM: Asiri sicaklik %.1f°C ve yuksek hiz\r\n", current_temp);
   
        SYSTEM_EmergencyPowerCut(); 
        BRAKES_EmergencyEngage();
        
        return;
    }
 
   // 4. ACİL: OMRON ENGEL TESPİTİ! - ✅ BURADA KARAR VERİLİYOR!
    if (OPTICS_IsEmergencyObstacleDetected()) {
        printf("🚨🚨 ACİL DURUM: Omron engel tespit edildi! Frenleme yapılıyor...\r\n");
        
        // ✅ ACTUATOR KONTROLÜ SADECE BURADA!
        SYSTEM_EmergencyPowerCut();
        BRAKES_EmergencyEngage();
        
        // Flag'i temizle (bir sonraki döngüde tekrar kontrol et)
        OPTICS_ClearEmergencyFlag();
        
        return; // Diğer kararları engelle
    }
    
    // 1. HIZ KONTROLÜ - Aşırı hız koruması
    if (g_system_state.velocity_mps > MAX_SAFE_SPEED) {
        float overspeed_ratio = (g_system_state.velocity_mps - MAX_SAFE_SPEED) / MAX_SAFE_SPEED;
        uint8_t brake_power = (uint8_t)(overspeed_ratio * 60.0f); // %0-60 fren
        
        BRAKE_SetForce(brake_power);
        printf("🚨 OTONOM FREN: %.1f m/s > %.1f m/s, Fren=%u%%\r\n", 
               g_system_state.velocity_mps, MAX_SAFE_SPEED, brake_power);
    }
    
    // 2. SICAKLIK KONTROLÜ - Motor/batarya soğutma
    if (current_temp > CRITICAL_TEMP) {
        // Kritik sıcaklık - acil yavaşlama
        target_speed_mps = target_speed_mps * 0.3f; // %70 yavaşla
        BRAKE_SetForce(50); // %50 fren
        printf("🔥 KRITIK SICAKLIK: %.1f°C, Hiz dusuruluyor\r\n", current_temp);
        
    } else if (current_temp > WARNING_TEMP) {
        // Uyarı sıcaklığı - kademeli yavaşlama  
        float reduction = 1.0f - ((current_temp - WARNING_TEMP) / (CRITICAL_TEMP - WARNING_TEMP)) * 0.5f;
        target_speed_mps = target_speed_mps * reduction;
        printf("⚠️  YUKSEK SICAKLIK: %.1f°C, Hiz %.0f%%\r\n", current_temp, reduction * 100.0f);
    }
    
    // 3. VOLTAJ KONTROLÜ - Batarya koruma
    if (current_voltage < CRITICAL_VOLTAGE) {
        // Kritik voltaj - acil durum
        target_speed_mps = 0.0f;
        BRAKE_SetForce(100);
        printf("🔋 KRITIK VOLTAJ: %.1fV, ACIL DURUM!\r\n", current_voltage);
        
    } else if (current_voltage < LOW_VOLTAGE_THRESHOLD) {
        // Düşük voltaj - yavaşla ve uyar
        target_speed_mps = target_speed_mps * 0.5f; // %50 yavaşla
        printf("⚠️  DUSUK VOLTAJ: %.1fV, Hiz yariya dusuruldu\r\n", current_voltage);
    }
    
    // 4. HEDEF HIZ KONTROLÜ - PID benzeri basit kontrol
    float speed_error = target_speed_mps - g_system_state.velocity_mps;
    
    if (speed_error < -2.0f) {
        // Çok hızlı - frenle (hedeften 2 m/s fazla)
        uint8_t brake_power = (uint8_t)(fabsf(speed_error) * 10.0f);
        if (brake_power > 70) brake_power = 70;
        BRAKE_SetForce(brake_power);
        
    } else if (speed_error > 2.0f) {
        // Çok yavaş - frenleri serbest bırak
        BRAKE_SetForce(0);
        printf("🚀 OTONOM HIZLANMA: Frenler serbest\r\n");
    }
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

        // ========================
        // 3. OTONOM KARARLAR - Fiziksel kontrol
        // ========================
        CONTROL_AutonomousDecisions();

        
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
