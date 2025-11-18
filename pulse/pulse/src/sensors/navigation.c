
// Core/Src/navigation.c

#include "navigation.h"
#include "shared_data.h"
#include <stdio.h>
#include <math.h>

// Global sistem durum yapısı
extern System_State_t g_system_state;

// Navigasyon (Kalman Filtresi) Durum Değişkenleri (Yer tutucu)
static float KF_position_estimate = 0.0f;
static float KF_velocity_estimate = 0.0f;

void NAVIGATION_Init(void)
{
    // Kalman Filtresi Başlangıç Değerleri
    KF_position_estimate = 0.0f;
    KF_velocity_estimate = 0.0f;
    printf("Navigasyon Sistemi Baslatildi.\r\n");
}


void NAVIGATION_Update(float dt)
{
    // Bu fonksiyon 10ms'de bir (veya MPU hızıyla) çağrılmalıdır.

    // 1. Girdileri Al (shared_data'dan okuma)
    float measured_accel = g_system_state.acceleration_mss; // MPU'dan (Gürültülü)
    float measured_velocity = g_system_state.velocity_mps;  // Encoder'dan (Kesin)

    // 2. KALMAN FİLTRESİ İŞLEMİ (Basitleştirilmiş Tahmin/Düzeltme Mantığı)
    // PREDİKSİYON (MPU İvmesi ile Konum/Hız Tahmini)
    KF_position_estimate += KF_velocity_estimate * dt + 0.5f * measured_accel * dt * dt;
    KF_velocity_estimate += measured_accel * dt;

    // DÜZELTME (Encoder Hızı ile Düzeltme) - Basit Lineer Düzeltme
    float velocity_error = measured_velocity - KF_velocity_estimate;

    // Kalman Kazancı (Örnek: Çok yüksek kazanç, encoder'a güçlü güven)
    float K_gain = 0.8f;

    // Hız Tahminini Düzelt
    KF_velocity_estimate += K_gain * velocity_error;

    // 3. Çıktıları shared_data'ya geri yaz
    g_system_state.position_m = KF_position_estimate;
    g_system_state.velocity_mps = KF_velocity_estimate; // Artık filtrelenmiş hız kullanılıyor

    // İvme değeri (MPU'dan gelen direkt değer kalır)
}

