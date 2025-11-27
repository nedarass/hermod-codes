// Core/Src/ntc_sensor.c

#include "ntc_sensor.h"
#include <math.h>
#include <stdio.h> // printf için

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
static float Convert_ADC_to_Temp(uint32_t adc_val)
{
    // 0 veya Max değer gelirse hesaplama yapma (Hata koruması)
    if (adc_val == 0 || adc_val >= 4095) return 999.0f; // Hata kodu olarak 999

    // 1. Voltaj Hesabı
    float v_out = (VREF * (float)adc_val) / ADC_MAX_VAL;

    // 2. Direnç Hesabı (R_ntc)
    // Devre: 3.3V -> R_FIXED -> [V_OUT] -> NTC -> GND varsayımıyla:
    // Formül: V_out = V_in * (R_ntc / (R_fixed + R_ntc))
    // Buradan R_ntc çekilirse: R_ntc = (V_out * R_fixed) / (V_in - V_out)
    
    float r_ntc = (v_out * R_FIXED) / (VREF - v_out);

    // 3. Sıcaklık Hesabı (Beta Formülü)
    // 1/T = 1/T0 + (1/B) * ln(R/R0)
    float ln_term = logf(r_ntc / R0);
    float inv_T = (1.0f / T0_K) + ((1.0f / BETA) * ln_term);
    
    // Kelvin -> Celsius
    return (1.0f / inv_T) - 273.15f;
}
// --- ANA FONKSİYONLAR ---

void NTC_Init(void)
{
    // Shared Data temizliği
    shared_data.sensors.battery.ntc_temp_c = 0.0f;
}


void NTC_Update(ADC_HandleTypeDef *hadc)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    // Handle kontrolü
    if (hadc == NULL) return;

    // 1. Kanalı Seç (Çoklu kanal kullanılıyorsa bu gereklidir)
    sConfig.Channel = NTC_ADC_CHANNEL_1;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_56CYCLES; //DAHA uzun örnekleme
    
    if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK)
    {
        // Kanal hatası
        shared_data.system.error_flags |= ERR_FLAG_NTC_OOR;
        return;
    }

// 2. ADC Okumasını Başlat
    HAL_ADC_Start(hadc);

    // 3. Çevrimin Bitmesini Bekle (Polling - CPU'yu kısa süre bekletir)
    if (HAL_ADC_PollForConversion(hadc, 10) == HAL_OK)
    {
        // 4. Değeri Oku
        uint32_t adc_raw = HAL_ADC_GetValue(hadc);
        
        // 4. Dönüştür
        float temp_c = Convert_ADC_to_Temp(adc_raw);

        // 5. Kaydet
        shared_data.sensors.battery.ntc_temp_c = temp_c;

        // 6. Limit Kontrolü (Güvenlik)
        if (temp_c > 85.0f) // Örn: 85 derece üstü tehlike
        {
             shared_data.system.error_flags |= ERR_FLAG_NTC_OOR;
             // İleride buraya "Fanları Aç" kodu eklenebilir.
        }
        else
        {
             // Hata bayrağını temizle (Bitwise AND ve NOT)
             shared_data.system.error_flags &= ~ERR_FLAG_NTC_OOR;
        }

// 7. ADC'yi Durdur (Güç tasarrufu ve temizlik için)
    HAL_ADC_Stop(hadc);
}
