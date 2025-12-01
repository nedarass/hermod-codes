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
#include "optics.h"  

// --- SÜRÜCÜ BAŞLIK DOSYALARI ---
#include "ntc_sensor.h"
#include "encoder.h"
#include "mpu9250.h"
#include "navigation.h"

// --- AKTÜATÖR SÜRÜCÜLERİ (Otonom kontrol için) ---
#include "actuators/brakes.h"      // Fren kontrolü
#include "actuators/vesc6.h"       // Motor kontrolü
#include "actuators/power_cut.h"   // Güç kesme kontrolü

#include <stdio.h> // printf için
#include <math.h>

// --- DONANIM HANDLE'LARI (Main.c'den gelir) ---
// Bu isimlerin CubeMX'teki tanımlarla birebir aynı olduğundan emin olun!
extern ADC_HandleTypeDef hadc1; // NTC Sensörü için
extern TIM_HandleTypeDef htim3; // Encoder için (Encoder Modunda)
extern I2C_HandleTypeDef hi2c1; // MPU9250 için

// --- AYARLAR ---
static const float EMERGENCY_SPEED = 90.0f;
static const float MAX_SAFE_SPEED = 80.0f;
static const float DT = 0.01f; // 10ms

// Sıcaklık eşikleri (Batarya/Motor)
static const float TEMP_WARNING = 60.0f;        // Uyarı: Hızı %30 düşür
static const float TEMP_CRITICAL = 75.0f;       // Kritik: Frenle + Yavaşla
static const float TEMP_SHUTDOWN = 85.0f;       

// Voltaj eşikleri (48V Lityum sistemi)
static const float VOLTAGE_WARNING = 44.0f;     // Uyarı: Güç %50
static const float VOLTAGE_CRITICAL = 42.0f;    // Kritik: Acil iniş
static const float VOLTAGE_SHUTDOWN = 38.0f;    

static uint32_t last_sensor_update = 0;
static uint32_t last_telemetry_tx = 0;
static uint32_t last_health_tx = 0;
static float target_speed_mps = 0.0f

// -------------------------------------------------------------------
// --- BAŞLATMA FONKSİYONU ---
// -------------------------------------------------------------------
void CONTROL_Init(void)
{
    NTC_Init();
    ENCODER_Init(&htim3);
    if (MPU9250_Init(&hi2c1) != HAL_OK) {
        shared_data.system.error_flags |= ERR_FLAG_MPU_FAIL;
    }
    NAVIGATION_Init();
    OPTICS_Init();
    COMM_Init();
    
    // Güvenlik: Frenleri kilitli başlat
    BRAKES_SetState(BRAKE_ENGAGED);
    
    printf("CONTROL: Sistem Hazir.\r\n");
}

// -------------------------------------------------------------------
// --- ACİL DURUM VE OTONOM KARARLAR ---
// -------------------------------------------------------------------

void CONTROL_AutonomousDecisions(void)
{
    
    // A. VERİLERİ OKU (Doğru Kaynaktan: Füzyon ve İşlenmiş Veriler)
    // -----------------------------------------------------------
    float velocity_mps = shared_data.sensors.nav.velocity_mps; // Kalman Filtreli Hız
    float temp_c       = shared_data.sensors.battery.ntc_temp_c;
    float voltage_v    = shared_data.sensors.battery.voltage_mv / 1000.0f;
    bool  obstacle     = OPTICS_IsEmergencyObstacleDetected();

   float effective_target_mps = target_speed_mps; // Efektif hedef hız

  // B. ACİL DURUM KONTROLLERİ (Safety First)
    // -----------------------------------------------------------
    bool emergency_trigger = false;
    char emergency_reason[50] = {0};

    
 
   
    
       // 1. Engel Tespiti (Optik Sensör) - EN YÜKSEK ÖNCELİK
    if (obstacle) {
        snprintf(emergency_reason, sizeof(emergency_reason), "Rayda Engel Tespit Edildi!");
        emergency_trigger = true;
        OPTICS_ClearEmergencyFlag();
    }
    // 2. Aşırı Hız
    else if (velocity_mps > EMERGENCY_SPEED) {
        snprintf(emergency_reason, sizeof(emergency_reason), "Asiri Hiz (%.1f m/s)!", velocity_mps);
        emergency_trigger = true;
    }
    // 3. Kritik Sıcaklık
    else if (temp_c > TEMP_SHUTDOWN) {
        snprintf(emergency_reason, sizeof(emergency_reason), "Kritik Sicaklik (%.1f C)!", temp_c);
        emergency_trigger = true;
    }
    // 4. Kritik Voltaj (1V üstü - sensör hatası değilse)
    else if (voltage_v < VOLTAGE_SHUTDOWN && voltage_v > 1.0f) {
        snprintf(emergency_reason, sizeof(emergency_reason), "Kritik Batarya Voltaji (%.1f V)!", voltage_v);
        emergency_trigger = true;
    }

    // ACİL DURUM TETİKLENDİYSE -> SİSTEMİ KAPAT
    if (emergency_trigger) {
        printf("🚨 ACIL DURUM: %s\r\n", emergency_reason);
        SYSTEM_EmergencyPowerCut();
        return; // Fonksiyondan çık, normal kontrol çalışmasın
    }

    // C. KADEMELİ KISITLAMALAR (Güvenlik Koridoru)

    // 1. HIZ KONTROLÜ - Aşırı hız koruması (Oransal Frenleme)
    if (velocity_mps > MAX_SAFE_SPEED) {
        float overspeed_ratio = (velocity_mps - MAX_SAFE_SPEED) / MAX_SAFE_SPEED;
        uint8_t brake_power = (uint8_t)(overspeed_ratio * 600.0f);
        
        if (brake_power > 100) brake_power = 100;
        if (brake_power < 10) brake_power = 10; // En az %10 fren
        
        BRAKE_SetForce(brake_power);
        effective_target_mps = 0; // Gazı kes
        
        printf("⚠️ OTONOM FREN: Hiz %.1f m/s (Limit: %.1f), Fren: %%%d\r\n", 
               velocity_mps, MAX_SAFE_SPEED, brake_power);
    }
    
    // 2. SICAKLIK KONTROLÜ - Kademeli güç düşürme
    if (temp_c > TEMP_CRITICAL) {
        // Kritik sıcaklık - Çok sert yavaşla
        effective_target_mps *= 0.3f; // Hedefi %70 düşür
        BRAKE_SetForce(50); // Yardımcı fren %50
        printf("⚠️ YUKSEK SICAKLIK: %.1fC -> Hiz %%30\r\n", temp_c);
    } 
    else if (temp_c > TEMP_WARNING) {
        // Uyarı sıcaklığı - Oransal yavaşlama
        float reduction = 1.0f - ((temp_c - TEMP_WARNING) / (TEMP_CRITICAL - TEMP_WARNING)) * 0.7f;
        effective_target_mps *= reduction;
        printf("⚠️ SICAKLIK UYARISI: %.1fC -> Hiz %%%.0f\r\n", temp_c, reduction * 100);
    }

    // 3. VOLTAJ KONTROLÜ - Pil ömrü koruma
    if (voltage_v < VOLTAGE_CRITICAL && voltage_v > 1.0f) {
        // Kritik voltaj - Acil iniş modu
        effective_target_mps *= 0.4f; // %40 güç
        printf("⚠️ KRITIK VOLTAJ: %.1fV -> Hiz %%40\r\n", voltage_v);
    }
    else if (voltage_v < VOLTAGE_WARNING && voltage_v > 1.0f) {
        // Düşük voltaj - Güç tasarrufu
        effective_target_mps *= 0.6f; // %60 güç
        printf("⚠️ DUSUK VOLTAJ: %.1fV -> Hiz %%60\r\n", voltage_v);
    }

    // D. HIZ KONTROLÜ - PID benzeri basit kontrol
    float speed_error = effective_target_mps - velocity_mps;

    // Senaryo 1: Hedefin çok üzerindeyiz (Yavaşlamalıyız)
    if (speed_error < -2.0f) {
        uint8_t brake_force = (uint8_t)(fabsf(speed_error) * 5.0f);
        if (brake_force > 100) brake_force = 100;
        if (brake_force < 5) brake_force = 5; // Minimum fren
        
        BRAKE_SetForce(brake_force);
        MOTOR_SetTargetSpeed(0); // Motoru boşa çıkar
        
        printf("📉 YAVASLAMA: Hata %.1f m/s, Fren: %%%d\r\n", speed_error, brake_force);
    } 
    // Senaryo 2: Hedefe yakınız veya altındayız (Hızlan/süzül)
    else {
        BRAKE_SetForce(0); // Frenleri bırak
        MOTOR_SetTargetSpeed(effective_target_mps); // Motoru güncellenmiş hedefle sür
        
        if (speed_error > 1.0f) {
            printf("HIZLANMA: Hedef %.1f, Mevcut %.1f\r\n", 
                   effective_target_mps, velocity_mps);
        }
    }
}

// ÇÖZÜM: Error handling ve recovery ekle
void CONTROL_HandleErrors(void)
{
    // MPU hatası - 5 saniyede bir yeniden dene
    if (shared_data.system.error_flags & ERR_FLAG_MPU_FAIL) {
        static uint32_t mpu_retry_time = 0;
        if (HAL_GetTick() - mpu_retry_time > 5000) {
            if (MPU9250_Init(&hi2c1) == HAL_OK) {
                shared_data.system.error_flags &= ~ERR_FLAG_MPU_FAIL;
                printf("✅ MPU9250 recovery successful\r\n");
            }
            mpu_retry_time = HAL_GetTick();
        }
    }
    
    // Communication hatası - Yeniden başlat
    if (shared_data.system.error_flags & ERR_FLAG_COMM_TIMEOUT) {
        printf("⚠️ COMM timeout, restarting...\r\n");
        COMM_Init();
        shared_data.system.error_flags &= ~ERR_FLAG_COMM_TIMEOUT;
    }
    
    // NTC sensör hatası
    if (shared_data.system.error_flags & ERR_FLAG_NTC_OOR) {
        // Sıcaklık normal seviyeye döndü mü kontrol et
        if (shared_data.sensors.battery.ntc_temp_c < TEMP_WARNING) {
            shared_data.system.error_flags &= ~ERR_FLAG_NTC_OOR;
            printf("✅ NTC sensor recovered\r\n");
        }
    }
}



// -------------------------------------------------------------------
// --- ANA KONTROL DÖNGÜSÜ ---
// -------------------------------------------------------------------
void CONTROL_Loop(void)
{
    uint32_t current_tick = HAL_GetTick();

    CONTROL_HandleErrors();
    // ============================================================
    // 1. SENSÖR OKUMA VE NAVİGASYON (Her 10ms'de bir)
    // ============================================================
    if (current_tick - last_sensor_update >= 10)
    {
      
        // 1. Sensörleri Oku
        NTC_Update(&hadc1);             // Sıcaklık
        ENCODER_Update(&htim3);         // Ham Hız
        MPU9250_Trigger_Read(&hi2c1);   // İvme (DMA Başlat)
        OPTICS_Update();                // Engel Kontrolü
        
        // 2. Sensör Füzyonu (Verileri Birleştir)
        NAVIGATION_Update(DT); 
        
        // 3. Karar Ver (Frenle, Dur veya Git)
        CONTROL_AutonomousDecisions();

        // Çalışma süresini güncelle
        shared_data.system.run_time_ms = current_tick;
        last_sensor_update = current_tick;
    }
   // ============================================================
    // B. 10Hz ÇEVRİMİ (Telemetri - Hızlı Veriler)
    // ============================================================
    if (current_tick - last_telemetry_tx >= 100)
    {
        // Hız, Konum, Voltaj vb. gönder
        COMM_SendTelemetryData();
        last_telemetry_tx = current_tick;
    }

    // ============================================================
    // C. 1Hz ÇEVRİMİ (Sağlık Raporu - Yavaş Veriler)
    // ============================================================
    if (current_tick - last_health_tx >= 1000)
    {
        // Sahte CPU sıcaklığı (Sensör eklenene kadar)
        shared_data.system.cpu_temp_c = 35.5f+ (rand() % 100) * 0.1f;
        
        // Hata bayrakları ve durum bilgisi gönder
        COMM_SendHealthCheck();
        last_health_tx = current_tick;
    }
}
