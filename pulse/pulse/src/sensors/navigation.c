// Core/Src/navigation.c

#include "navigation.h"
#include "shared_data.h"
#include <stdio.h>
#include <math.h>

// Global sistem durum yapısı
extern System_State_t g_system_state;

// --- KALMAN FİLTRESİ PARAMETRELERİ (AYARLANABİLİR) ---

// Q: Sistemin (İvme/Fizik Modelinin) Güvensizliği
// Bu değer artarsa filtre, Encoder ölçümüne daha çok güvenir.
#define KF_Q_ACCEL  0.001f 

// R: Ölçümün (Encoder'ın) Gürültüsü
// Bu değer artarsa filtre, İvme integraline (tahmine) daha çok güvenir.
#define KF_R_ENCODER 0.1f

// --- NAVİGASYON DURUM DEĞİŞKENLERİ ---
typedef struct {
    float position_m;      // Tahmini Konum
    float velocity_mps;    // Tahmini Hız
    float bias_accel;      // İvmeölçer Sabit Hatası (Bias)
    
    // Hata Kovaryans Matrisi (Sistemin kendine güveni)
    float P[2][2]; 
} Navigation_State_t;

static Navigation_State_t nav_state;


void NAVIGATION_Init(void)
{
    // Değişkenleri Sıfırla
    nav_state.position_m = 0.0f;
    nav_state.velocity_mps = 0.0f;
    nav_state.bias_accel = 0.0f;

    // Başlangıç Kovaryans (P) matrisi
    nav_state.P[0][0] = 1.0f;
    nav_state.P[0][1] = 0.0f;
    nav_state.P[1][0] = 0.0f;
    nav_state.P[1][1] = 1.0f;

    printf("Navigasyon Sistemi (Kalman) Baslatildi.\r\n");
}


void NAVIGATION_Update(float dt)
{
    // ============================================================
    // 1. GİRDİLERİ AL
    // ============================================================
    float input_accel = g_system_state.acceleration_mss; 
    float measured_velocity = g_system_state.velocity_mps; 

    // ============================================================
    // 2. TAHMİN (PREDICTION) - Fizik Modeli
    // ============================================================
    // İvme hatasını (bias) çıkararak saf ivmeyi bul
    float accel_net = input_accel - nav_state.bias_accel;
    
    // Fizik formülleri: x = x + v*t + 0.5*a*t^2
    nav_state.position_m   += nav_state.velocity_mps * dt + 0.5f * accel_net * dt * dt;
    nav_state.velocity_mps += accel_net * dt;

    // Hata matrisini (P) büyüt (Zaman geçtikçe belirsizlik artar)
    nav_state.P[0][0] += KF_Q_ACCEL; 
    nav_state.P[1][1] += KF_Q_ACCEL;

    // ============================================================
    // 3. DÜZELTME (CORRECTION) - Encoder ile
    // ============================================================
    
    // Fark: Encoder hızı ile bizim tahmin ettiğimiz hız arasındaki fark
    float velocity_error = measured_velocity - nav_state.velocity_mps;

    // Kalman Kazancı (K): Otomatik hesaplanır
    // Eğer Encoder çok gürültülüyse K küçülür, filtre ivmeye güvenir.
    float S = nav_state.P[1][1] + KF_R_ENCODER; 
    float K_gain = nav_state.P[1][1] / S;

    // Durumu Düzelt
    nav_state.velocity_mps += K_gain * velocity_error;
    
    // Bias Tahminini Güncelle (Basit yaklaşım: Hata varsa bias vardır)
    // Not: Bias'ı yavaşça öğrenir (0.01 katsayısı ile)
    nav_state.bias_accel += 0.01f * K_gain * velocity_error; 
    
    // Hata matrisini (P) küçült (Ölçüm geldiği için belirsizlik azaldı)
    nav_state.P[1][1] = (1.0f - K_gain) * nav_state.P[1][1];


    // ============================================================
    // 4. ÇIKTILARI YAZ
    // ============================================================
    g_system_state.position_m = nav_state.position_m;
    // Filtrelenmiş "akıllı" hız değerini yazıyoruz:
    g_system_state.velocity_mps = nav_state.velocity_mps; 
}
