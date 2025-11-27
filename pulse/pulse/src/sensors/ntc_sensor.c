// Core/Src/ntc_sensor.c

#include "ntc_sensor.h"
#include "shared_data.h" // Shared Data erişimi için eklendi
#include <math.h>
#include <stdio.h> 

// --- AYARLAR ---
// LÜTFEN STM32CubeMX'te KULLANILAN PİNİN KANALINI KONTROL EDİNİZ.
#define NTC_ADC_CHANNEL_1    ADC_CHANNEL_8

// NTC Sabitleri
#define VREF                 3.3f    // Referans Voltajı
#define ADC_MAX              4095.0f // 12-bit ADC Çözünürlüğü
#define R_FIXED              10000.0f // Gerilim bölücüdeki sabit direnç (10k)
#define R0                   10000.0f // NTC'nin 25°C'deki direnci (10k)
#define T0_K                 298.15f // 25°C'nin Kelvin karşılığı
#define BETA                 3950.0f // NTC Beta katsayısı

// --- YARDIMCI FONKSİYONLAR ---

// Voltajdan NTC direncini ve Sıcaklığı hesaplar
static float Convert_ADC_to_Temp(uint32_t adc_val)
{
    // 0 veya Max değer gelirse hesaplama yapma (Kablo kopuk veya kısa devre)
    if (adc_val == 0 || adc_val >= 4095) return 999.0f; // Hata değeri

    // 1. Voltaj Hesabı (DÜZELTME: ADC_MAX_VAL -> ADC_MAX yapıldı)
    float v_out = (VREF * (float)adc_val) / ADC_MAX;

    // 2. Direnç Hesabı (R_ntc)
    // Devre: 3.3V -> R_FIXED -> [V_OUT] -> NTC -> GND varsayımıyla:
    // Formül: R_ntc = (V_out * R_fixed) / (V_in - V_out)
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

    // 1. Kanalı Konfigüre Et 
    // (Eğer birden fazla sensör aynı ADC'yi kullanıyorsa bu adım zorunludur)
    sConfig.Channel = NTC_ADC_CHANNEL_1;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_56CYCLES_5; // ADC okuma süresi (stm32f1 için düzeltildi)
    
    if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK)
    {
        
        shared_data.system.error_flags |= ERR_NTC_OOR;
        return;
    }

    // 2. ADC Okumasını Başlat
    HAL_ADC_Start(hadc);

    // 3. Çevrimin Bitmesini Bekle (Polling - Basit Yöntem)
    if (HAL_ADC_PollForConversion(hadc, 10) == HAL_OK)
    {
        // 4. Değeri Oku
        uint32_t adc_raw = HAL_ADC_GetValue(hadc);
        
        // 5. Dönüştür
        float temp_c = Convert_ADC_to_Temp(adc_raw);

        // 6. Kaydet
        shared_data.sensors.battery.ntc_temp_c = temp_c;

        // 7. Limit Kontrolü (Güvenlik)
        if (temp_c > 85.0f) // Örn: 85 derece üstü tehlike
        {
             
             shared_data.system.error_flags |= ERR_NTC_OOR;
        }
        else
        {
             // Hata bayrağını temizle
             shared_data.system.error_flags &= ~ERR_NTC_OOR;
        }
    }

    // 8. ADC'yi Durdur
    HAL_ADC_Stop(hadc);
}
