/*
// Core/Src/ntc_sensor.cpp

#include "ntc_sensor.h"
#include "shared_data.h"
#include <cmath>
#include <cstdio> // printf için (C++ uyumlu)

// Sistem Durum yapısı
extern System_State_t g_system_state;

// Sabitler (Lütfen kullandığınız NTC'ye göre güncelleyin)
constexpr float VREF = 3.3f;
constexpr float ADC_MAX = 4095.0f; // 12-bit ADC için
constexpr float R_FIXED = 10000.0f; // Sabit direnç (10k Ohm)
constexpr float R0 = 10000.0f; // 25C'de NTC direnci
constexpr float T0_K = 298.15f; // 25C Kelvin
constexpr float BETA = 3950.0f;

// Sadece NTC okuması için kullanılan ADC kanalı
// LÜTFEN STM32CubeMX'te KULLANILAN PİNİN KANALINI BURAYA YAZINIZ.
#define NTC_ADC_CHANNEL_1   ADC_CHANNEL_8

// C++ ile inline olarak daha verimli hale getirildi
float adc_to_voltage(uint32_t adc) {
    return (VREF * (float)adc) / ADC_MAX;
}

float v_to_ntc_resistance(float vout) {
    if (VREF - vout <= 1e-6f) return 1e6f;
    return (R_FIXED * vout) / (VREF - vout);
}

float ntc_resistance_to_temp_c(float r_ntc) {
    // Beta formülü
    float invT = (1.0f / T0_K) + (1.0f / BETA) * std::log(r_ntc / R0);
    float tempK = 1.0f / invT;
    return tempK - 273.15f;
}

// C Arayüz Fonksiyonları (C++'ta tanımlanır)

void NTC_Init(void) {
    // Sensör için özel başlatma kodu (gerekirse)
}

void NTC_Update(ADC_HandleTypeDef *hadc)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel      = NTC_ADC_CHANNEL_1;
    sConfig.Rank         = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;

    // ADC Handle'ı kontrol et
    if (hadc == nullptr) return;

    HAL_ADC_Stop(hadc);
    if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK) {
        g_system_state.error_flags |= ERR_FLAG_NTC_OOR;
        return;
    }

    if (HAL_ADC_Start(hadc) != HAL_OK) return;
    if (HAL_ADC_PollForConversion(hadc, 10) != HAL_OK) return;

    uint32_t adc_raw = HAL_ADC_GetValue(hadc);
    HAL_ADC_Stop(hadc);

    float v_out = adc_to_voltage(adc_raw);
    float r_ntc = v_to_ntc_resistance(v_out);
    float temp_c = ntc_resistance_to_temp_c(r_ntc);

    // Santigrat * 100 olarak Int16'ya kaydet
    g_system_state.raw_temp_ntc1 = (int16_t)(temp_c * 100.0f);

    if (temp_c > 100.0f) { // Örnek Yüksek Sıcaklık Limiti
        g_system_state.error_flags |= ERR_FLAG_NTC_OOR;
    } else {
        g_system_state.error_flags &= ~ERR_FLAG_NTC_OOR;
    }
}
*/// Core/Src/ntc_sensor.c

#include "ntc_sensor.h"
#include <math.h>
#include <stdio.h> // printf için

// Harici HAL tanimi
extern ADC_HandleTypeDef hadc1;

// Sistem Durum yapisi
extern System_State_t g_system_state;

// Sadece NTC okuması için kullanılan ADC kanalı (Örnek: ADC_CHANNEL_8 -> PB0 pini)
// LÜTFEN STM32CubeMX'te KULLANILAN PİNİN KANALINI BURAYA YAZINIZ.
#define NTC_ADC_CHANNEL_1   ADC_CHANNEL_8

// NTC Sabitleri (ntc_sensor.h'den gelir)
#define VREF                 3.3f
#define ADC_MAX              4095.0f
#define R_FIXED              10000.0f
#define R0                   10000.0f
#define T0_K                 298.15f
#define BETA                 3950.0f

// Tek seferlik kanal okuma (Polling)
static uint32_t read_adc_once(uint32_t channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel      = channel;
    sConfig.Rank         = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;

    HAL_ADC_Stop(&hadc1);
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        g_system_state.error_flags |= ERR_FLAG_NTC_OOR;
        return 0;
    }

    if (HAL_ADC_Start(&hadc1) != HAL_OK) return 0;
    if (HAL_ADC_PollForConversion(&hadc1, 10) != HAL_OK) return 0;

    uint32_t val = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return val;
}

static float v_to_ntc_resistance(float vout)
{
    // Rntc = Rfixed * Vout / (Vref - Vout)
    if (VREF - vout <= 1e-6f) return 1e6f;
    return (R_FIXED * vout) / (VREF - vout);
}

static float ntc_resistance_to_temp_c(float r_ntc)
{
    // Beta formülü: 1/T = 1/T0 + (1/B)*ln(R/R0)
    float invT = (1.0f / T0_K) + (1.0f / BETA) * logf(r_ntc / R0);
    float tempK = 1.0f / invT;
    return tempK - 273.15f;
}

// NTC Sensorunu okur ve global veri yapısını günceller.
void NTC_Update(void)
{
    uint32_t adc_raw = read_adc_once(NTC_ADC_CHANNEL_1);
    if (adc_raw == 0) return;

    float v_out = (VREF * (float)adc_raw) / ADC_MAX;
    float r_ntc = v_to_ntc_resistance(v_out);
    float temp_c = ntc_resistance_to_temp_c(r_ntc);

    // Değeri Santigrat * 100 olarak Int16'ya kaydet (Telemetriye uygun format)
    g_system_state.raw_temp_ntc1 = (int16_t)(temp_c * 100.0f);

    if (temp_c > 85.0f || temp_c < -10.0f) {
        g_system_state.error_flags |= ERR_FLAG_NTC_OOR;
    } else {
        g_system_state.error_flags &= ~ERR_FLAG_NTC_OOR;
    }
}

void NTC_Init(void)
{
    // Gerekirse ilk okuma yapilabilir
}
