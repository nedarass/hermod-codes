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
static const float CRITICAL_TEMP = 75.0f;
static const float CRITICAL_VOLTAGE_V = 42.0f;
static const float DT = 0.01f; // 10ms

static uint32_t last_sensor_update = 0;
static uint32_t last_telemetry_tx = 0;
static uint32_t last_health_tx = 0;
static float target_speed_mps = 0.0f;
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
void CONTROL_AutonomousDecisions(void)
{
    
    // A. VERİLERİ OKU (Doğru Kaynaktan: Füzyon ve İşlenmiş Veriler)
    // -----------------------------------------------------------
    float velocity_mps = shared_data.sensors.nav.velocity_mps; // Kalman Filtreli Hız
    float temp_c       = shared_data.sensors.battery.ntc_temp_c;
    float voltage_v    = shared_data.sensors.battery.voltage_mv / 1000.0f;
    bool  obstacle     = OPTICS_IsEmergencyObstacleDetected();

  // B. ACİL DURUM KONTROLLERİ (Safety First)
    // -----------------------------------------------------------
    bool emergency_trigger = false;

    // 1. Aşırı Hız
    if (velocity_mps > EMERGENCY_SPEED) {
        printf("🚨 ACIL: Asiri Hiz (%.1f m/s)!\r\n", velocity_mps);
        emergency_trigger = true;
    }
    // 2. Kritik Düşük Voltaj (1V altı sensör hatasıdır, onu yoksay)
    else if (voltage_v < CRITICAL_VOLTAGE_V && voltage_v > 1.0f) {
        printf("🚨 ACIL: Kritik Batarya Voltaji (%.1f V)!\r\n", voltage_v);
        emergency_trigger = true;
    }
    // 3. Aşırı Sıcaklık
    else if (temp_c > CRITICAL_TEMP) {
        printf("🚨 ACIL: Batarya/Motor Asiri Isindi (%.1f C)!\r\n", temp_c);
        emergency_trigger = true;
    }
 
   // 4. Engel Tespiti (Optik Sensör)
    else if (obstacle) {
        printf("🚨 ACIL: Rayda Engel Tespit Edildi!\r\n");
        emergency_trigger = true;
        OPTICS_ClearEmergencyFlag(); // Flag'i temizle ki sürekli tetiklemesin
    }

    // Eğer herhangi bir acil durum varsa -> SİSTEMİ KAPAT
    if (emergency_trigger) {
        SYSTEM_EmergencyPowerCut(); // commands.c içindeki fonksiyon (Güç kes + Frenle + Logla)
        return; // Fonksiyondan çık, gaz verme kodu çalışmasın
    }
 
    // Normal Sürüş Protokolu 
 
    // A. HIZ KONTROLÜ - Aşırı hız koruması (Oransal Frenleme)
    if (velocity_mps > MAX_SAFE_SPEED) 
    {
        // Ne kadar hızlıyız? Orana göre fren şiddetini ayarla.
        // Örn: 88 m/s gidiyoruz (Sınır 80). Fark 8. Oran = 0.1 (%10).
        // Fren Gücü = 0.1 * 600 = 60 (Maks %60 fren uygula ki tekerlek kilitlenmesin)
        
        float overspeed_ratio = (velocity_mps - MAX_SAFE_SPEED) / MAX_SAFE_SPEED;
        uint8_t brake_power = (uint8_t)(overspeed_ratio * 600.0f); // Katsayıyı artırdım
        
        if (brake_power > 100) brake_power = 100;
        if (brake_power < 10) brake_power = 10; // En az %10 dokun

        BRAKE_SetForce(brake_power);
        MOTOR_SetTargetSpeed(0); // Gazı kes
        
        printf("⚠️ OTONOM FREN: Hiz %.1f, Fren Gücü %%%d\r\n", velocity_mps, brake_power);
        return; // Hızlanma koduna girme
    }
    
    // B. SICAKLIK KONTROLÜ (Isınmaya Göre Yavaşlama)
    if (current_temp > CRITICAL_TEMP) 
    {
        // Kritik sıcaklık - Çok sert yavaşla
        effective_target_mps = effective_target_mps * 0.3f; // Hedefi %70 düşür
        BRAKE_SetForce(50); // Yardımcı fren
    } 
    else if (current_temp > WARNING_TEMP) 
    {
        // Uyarı sıcaklığı - Lineer (Oransal) yavaşlama
        // 60C -> %100 Hız, 75C -> %50 Hız gibi bir rampa oluşturuyoruz.
        float reduction_factor = 1.0f - ((current_temp - WARNING_TEMP) / (CRITICAL_TEMP - WARNING_TEMP)) * 0.5f;
        
        effective_target_mps = effective_target_mps * reduction_factor;
    }
 
   // C. VOLTAJ KONTROLÜ (Pil Tasarrufu)
    if (current_volts < LOW_VOLTAGE_THRESHOLD && current_volts > 1.0f) 
    {
        effective_target_mps = effective_target_mps * 0.5f; // %50 Güç tasarrufu
    }
    
    // 4. HEDEF HIZ KONTROLÜ - PID benzeri basit kontrol
    float speed_error = effective_target_mps - velocity_mps;

    // Senaryo 1: Hedefin çok üzerindeyiz (Yavaşlamalıyız)
    if (speed_error < -2.0f) 
    {
        // Hata ne kadar büyükse o kadar sert fren yap
        // Örn: Fark 5 m/s -> Fren 5 * 5 = %25
        uint8_t brake_force = (uint8_t)(fabsf(speed_error) * 5.0f);
        if (brake_force > 100) brake_force = 100;
        
        BRAKE_SetForce(brake_force);
        MOTOR_SetTargetSpeed(0); // Motoru boşa çıkar
    } // Senaryo 2: Hedefe yakınız veya altındayız (Hızlanmalıyız veya Süzülmeliyiz)
    else 
    {
        // Frenleri bırak
        BRAKE_SetForce(0);

        // Motor sürücüsüne YENİ (limite takılmış/düşürülmüş) hedefi gönder
        // Eğer sıcaklık/voltaj limitleri hızı düşürdüyse, motor buradaki yeni değeri alacak.
        MOTOR_SetTargetSpeed(effective_target_mps);
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
        shared_data.system.cpu_temp_c = 35.5f; 
        
        // Hata bayrakları ve durum bilgisi gönder
        COMM_SendHealthCheck();
        last_health_tx = current_tick;
    }
}
