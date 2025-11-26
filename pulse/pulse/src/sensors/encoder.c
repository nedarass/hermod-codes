//HAL_TIM_Encoder_Start, __HAL_TIM_GET_COUNTER: Bunlar STM32 ile birlikte gelir.

#include "encoder.h"
#include "shared_data.h"
#include <stdio.h> // printf için

// Global sistem durum yapısı (Veriyi buraya yazacağız)
extern System_State_t g_system_state;

// --- AYARLANABİLİR SABİTLER ---
// Bu değerleri kullandığın donanıma (Encoder ve Tekerlek) göre değiştirmelisin!
#define ENCODER_COUNTS_PER_REV  4096.0f // Encoder'ın bir tam turu için ürettiği toplam sayım (Pulse/Rev)
#define WHEEL_CIRCUMFERENCE_M   0.5f    // Tekerleğin çevresi (Metre cinsinden)
#define UPDATE_PERIOD_SEC       0.01f   // Hız hesaplaması için bu fonksiyonun ne sıklıkla çağrıldığı (Saniye)

// --- DEĞİŞKENLER ---
static int32_t last_counter_value = 0;    // Bir önceki okumadaki sayaç değeri
static int64_t total_pulse_count = 0;     // Toplam katedilen mesafe (pulse cinsinden)


// --- FONKSİYONLAR ---

void ENCODER_Init(TIM_HandleTypeDef *htim)
{
    // Timer'ı Encoder modunda başlat
    if (HAL_TIM_Encoder_Start(htim, TIM_CHANNEL_ALL) != HAL_OK)
    {
        // Başlatma hatası olursa hata bayrağı dikilebilir
        // g_system_state.error_flags |= ...
        printf("Encoder Baslatma Hatasi!\r\n");
        return;
    }

    // Başlangıç değerini al (Sıfır olması beklenir ama güvenli olsun)
    last_counter_value = (int32_t)__HAL_TIM_GET_COUNTER(htim);
    
    printf("Encoder Baslatildi.\r\n");
}

void ENCODER_Update(TIM_HandleTypeDef *htim)
{
    // 1. Sayacın güncel değerini donanımdan oku
    uint32_t current_counter_value = __HAL_TIM_GET_COUNTER(htim);

    // 2. Değişimi (Delta) Hesapla
    int16_t delta_counts = (int16_t)(current_counter_value - (uint32_t)last_counter_value);

    // 3. Toplam Mesafeyi Güncelle (Absolute Position)
    total_pulse_count += delta_counts;

    // 4. Fiziksel Hesaplamalar
    
    // a) Anlık Hız (Metre / Saniye)
    // Hız = (Delta Pulse / Tur Başına Pulse) * Tekerlek Çevresi / Zaman
    float velocity_mps = ((float)delta_counts / ENCODER_COUNTS_PER_REV) * WHEEL_CIRCUMFERENCE_M / UPDATE_PERIOD_SEC;

    // b) Toplam Konum (Metre)
    float position_m = ((float)total_pulse_count / ENCODER_COUNTS_PER_REV) * WHEEL_CIRCUMFERENCE_M;

    // 5. Verileri Global Yapıya (Shared Data) Yaz
    g_system_state.velocity_mps = velocity_mps;
    g_system_state.position_m   = position_m;

    // 6. Son değeri sakla (Bir sonraki döngü için)
    last_counter_value = current_counter_value;
}
