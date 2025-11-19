// Core/Src/ntc_sensor.c

#include "ntc_sensor.h"
#include <math.h>
#include <stdio.h> // printf için

// Sistem Durum yapisi ( veriyi buraya yazacağız)
extern System_State_t g_system_state;

// Sadece NTC okuması için kullanılan ADC kanalı (Örnek: ADC_CHANNEL_8 -> PB0 pini)
// LÜTFEN STM32CubeMX'te KULLANILAN PİNİN KANALINI BURAYA YAZINIZ.
#define NTC_ADC_CHANNEL_1   ADC_CHANNEL_8

// NTC Sabitleri (ntc_sensor.h'den gelir)
// Devre tasarımına göre bu değerleri kontrol et!
#define VREF                 3.3f // Referans Voltajı
#define ADC_MAX              4095.0f // 12-bit ADC Çözünürlüğü
#define R_FIXED              10000.0f // Gerilim bölücüdeki sabit direnç (10k)
#define R0                   10000.0f // NTC'nin 25°C'deki direnci (10k)
#define T0_K                 298.15f // 25°C'nin Kelvin karşılığı
#define BETA                 3950.0f // NTC Beta katsayısı

// --- YARDIMCI MATEMATİK FONKSİYONLARI ---

// ADC değerini Voltaja çevirir
static inline float adc_to_voltage(uint32_t adc)
{
    return (VREF * (float)adc) / ADC_MAX;
}
// Voltajdan NTC direncini hesaplar (Gerilim Bölücü Formülü)
static float v_to_ntc_resistance(float vout)
{
    // Bölme işleminde sıfıra bölünmeyi engelle
    if ((VREF - vout) <= 0.001f) return 1000000.0f; // Çok yüksek direnç (soğuk)
    if (vout <= 0.001f) return 1.0f;                // Çok düşük direnç (sıcak)

    // Devre Yapısı: [3.3V] -- [R_FIXED] -- (Vout) -- [NTC] -- [GND] varsayımıyla:
    // Vout = Vref * Rntc / (Rfixed + Rntc) formülünden çekilmiştir:
    return (R_FIXED * vout) / (VREF - vout);
}
// Dirençten Sıcaklığı (Celsius) hesaplar (Beta Formülü)
static float ntc_resistance_to_temp_c(float r_ntc)
{
    // 1/T = 1/T0 + (1/B) * ln(R/R0)
    float ln_term = logf(r_ntc / R0);
    float invT = (1.0f / T0_K) + ((1.0f / BETA) * ln_term);
    
    // Kelvin'den Celsius'a çevir
    return (1.0f / invT) - 273.15f;
}
// --- ANA FONKSİYONLAR ---

void NTC_Init(void)
{
    // Şu anlık özel bir başlatma gerekmiyor, ADC main.c'de başlatılıyor.
}

// control.c içerisinden çağrılır.
void NTC_Update(ADC_HandleTypeDef *hadc)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    // Handle kontrolü
    if (hadc == NULL) return;

    // 1. Kanalı Seç (Çoklu kanal kullanılıyorsa bu gereklidir)
    sConfig.Channel = NTC_ADC_CHANNEL_1;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5; // Orta hızda örnekleme
    
    if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK)
    {
        g_system_state.error_flags |= ERR_FLAG_NTC_OOR;
        return;
    }

// 2. ADC Okumasını Başlat
    HAL_ADC_Start(hadc);

    // 3. Çevrimin Bitmesini Bekle (Polling - CPU'yu kısa süre bekletir)
    if (HAL_ADC_PollForConversion(hadc, 10) == HAL_OK)
    {
        // 4. Değeri Oku
        uint32_t adc_raw = HAL_ADC_GetValue(hadc);
        
        // 5. Hesaplamaları Yap
        float v_out = adc_to_voltage(adc_raw);
        float r_ntc = v_to_ntc_resistance(v_out);
        float temp_c = ntc_resistance_to_temp_c(r_ntc);

        // 6. Veriyi Global Yapıya Yaz (Örn: 25.45 C -> 2545)
        // g_System_state raw_temp_ntc1 shared datada kayıtlı 
        g_system_state.raw_temp_ntc1 = (int16_t)(temp_c * 100.0f);

        // Limit Kontrolü (Hata Bayrakları)
        if (temp_c > 85.0f || temp_c < -20.0f) {
             g_system_state.error_flags |= ERR_FLAG_NTC_OOR;
        } else {
             g_system_state.error_flags &= ~ERR_FLAG_NTC_OOR;
        }
    }
    else
    {
        // Okuma başarısız
        g_system_state.error_flags |= ERR_FLAG_NTC_OOR;
    }

// 7. ADC'yi Durdur (Güç tasarrufu ve temizlik için)
    HAL_ADC_Stop(hadc);
}
