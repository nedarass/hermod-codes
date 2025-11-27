// pulse/pulse/src/sensors/navigation.c
#include "navigation.h"
#include "shared_data.h"
#include <stdio.h>
#include <math.h>

// --- KALMAN FİLTRESİ PARAMETRELERİ (AYARLANABİLİR) ---

// Q: Sistemin (İvme/Fizik Modelinin) Güvensizliği
// Bu değer artarsa filtre, Encoder ölçümüne daha çok güvenir.
#define KF_Q_ACCEL  0.01f  // (Biraz artırdım, ivme gürültüsüne tolerans için)

// R: Ölçümün (Encoder'ın) Gürültüsü
// Bu değer artarsa filtre, İvme integraline (tahmine) daha çok güvenir.
#define KF_R_ENCODER 0.5f  // (Titreşimden dolayı encoder hatasını yüksek varsayıyoruz)

// --- NAVİGASYON DURUM DEĞİŞKENLERİ ---
typedef struct {
    float position_m;      // Tahmini Konum
    float velocity_mps;    // Tahmini Hız
    float bias_accel;      // İvmeölçer Sabit Hatası (Bias) - Statik olarak belirlenecek
    
    // Hata Kovaryans Matrisi (Sistemin kendine güveni)
    // DEĞİŞİKLİK: 2x2 Matris yerine işlemci dostu tekil değişken kullanıyoruz.
    float P_vel; 
} Navigation_State_t;

static Navigation_State_t nav_state;

// Kalibrasyon için sayaçlar (YENİ)
static uint16_t calibration_counter = 0;
static float calibration_sum = 0.0f;
#define CALIBRATION_SAMPLES 100 // İlk 100 döngü (yaklaşık 1 sn) kalibrasyon yap

void NAVIGATION_Init(void)
{
    // Değişkenleri Sıfırla
    nav_state.position_m = 0.0f;
    nav_state.velocity_mps = 0.0f;
    nav_state.bias_accel = 0.0f;

    // Başlangıç Kovaryans (P) değeri
    nav_state.P_vel = 1.0f; // Başlangıçta sisteme tam güvenmiyoruz

    // Shared Data temizliği
    shared_data.sensors.nav.position_m = 0.0f;
    shared_data.sensors.nav.velocity_mps = 0.0f;
    
    // Kalibrasyonu sıfırla
    calibration_counter = 0;
    calibration_sum = 0.0f;
    
    printf("Navigasyon: Baslatildi. Kalibrasyon bekleniyor...\r\n");
}

// Bu fonksiyon main loop içinde dt süresiyle (örn: 0.01sn) çağrılmalı
void NAVIGATION_Update(float dt)
{
    // GÜVENLİK: dt hatalı gelirse işlem yapma
    if (dt <= 0.0001f || dt > 0.5f) return;

    // ============================================================
    // 1. GİRDİLERİ AL
    // ============================================================
    // İvme Kaynağı: MPU9250 X Ekseni (Podun gidiş yönü)
    float input_accel = shared_data.sensors.mpu.accel_x_mss;
    
    // Hız Kaynağı: Encoder (Filtrelenmiş veya Ham kullanılabilir)
    float measured_velocity = shared_data.sensors.encoder.velocity_mps;

    // --- ÖZEL BÖLÜM: BAŞLANGIÇ KALİBRASYONU (YENİ) ---
    // Araç hareket etmeden önce sensörün "Sıfır" hatasını öğreniyoruz.
    if (calibration_counter < CALIBRATION_SAMPLES) {
        calibration_sum += input_accel;
        calibration_counter++;
        
        if (calibration_counter == CALIBRATION_SAMPLES) {
            nav_state.bias_accel = calibration_sum / CALIBRATION_SAMPLES;
            // Bu değeri shared_data'ya da yazalım ki debug edebilelim
            shared_data.sensors.nav.accel_bias_mss = nav_state.bias_accel;
            printf("Navigasyon: Kalibrasyon Bitti. Bias: %f\r\n", nav_state.bias_accel);
        }
        return; // Kalibrasyon bitene kadar hesap yapma
    }

    // ============================================================
    // 2. TAHMİN (PREDICTION- TIME UPDATE) - Fizik Modeli
    // ============================================================
    // İvme hatasını (bias) çıkararak saf ivmeyi bul
    float accel_net = input_accel - nav_state.bias_accel;
    
    // Konum Tahmini: x = x + v*t + 0.5*a*t^2
    nav_state.position_m += (nav_state.velocity_mps * dt) + (0.5f * accel_net * dt * dt);

    // Hız Tahmini: v = v + a*t
    nav_state.velocity_mps += accel_net * dt;
    
    // Hata belirsizliğini (P) büyüt (Zaman geçtikçe belirsizlik artar)
    nav_state.P_vel += KF_Q_ACCEL * dt; 

    // ============================================================
    // 3. DÜZELTME (CORRECTION - MEASUREMENT UPDATE) - Encoder ile
    // ============================================================
    
    // Fark: Encoder hızı ile bizim tahmin ettiğimiz hız arasındaki fark
    float velocity_error = measured_velocity - nav_state.velocity_mps;

    // Kalman Kazancı (K): Otomatik hesaplanır
    // Eğer Encoder çok gürültülüyse K küçülür, filtre ivmeye güvenir.
    float S = nav_state.P_vel + KF_R_ENCODER; 
    float K_gain = nav_state.P_vel / S;

    // Durumu Düzelt (Hızı encoder verisine göre güncelle)
    nav_state.velocity_mps += K_gain * velocity_error;
    
    // Konumu da hızdaki düzeltme kadar kaydır (Bu daha doğru sonuç verir)
    nav_state.position_m += K_gain * velocity_error * dt;
    
    // NOT: Bias güncelleme satırı güvenlik nedeniyle kaldırıldı.
    // nav_state.bias_accel += ... (RİSKLİ)
    
    // KOVARYANS GÜNCELLEME (Ölçüm geldiği için belirsizlik azaldı)
    nav_state.P_vel = (1.0f - K_gain) * nav_state.P_vel;


    // ============================================================
    // 4. SONUÇLARI YAZ (Shared Data'ya)
    // ============================================================
    
    // Artık "encoder hızı" değil, "Füzyon Hızı"nı sisteme sunuyoruz.
    // Fren ve Motor algoritmaları ARTIK BU VERİYİ KULLANMALI!
    
    shared_data.sensors.nav.position_m     = nav_state.position_m;
    shared_data.sensors.nav.velocity_mps   = nav_state.velocity_mps;
    // Bias değerini sabit tutuyoruz
    shared_data.sensors.nav.accel_bias_mss = nav_state.bias_accel;
}


















/*#include "navigation.h"
#include "shared_data.h"
#include <stdio.h>
#include <math.h>

// --- KALMAN FİLTRESİ PARAMETRELERİ (AYARLANABİLİR) ---

// Q: Sistemin (İvme/Fizik Modelinin) Güvensizliği
// Bu değer artarsa filtre, Encoder ölçümüne daha çok güvenir.
#define KF_Q_ACCEL  0.005f 

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

    // Shared Data temizliği
    shared_data.sensors.nav.position_m = 0.0f;
    shared_data.sensors.nav.velocity_mps = 0.0f;
}

// Bu fonksiyon main loop içinde dt süresiyle (örn: 0.01sn) çağrılmalı
void NAVIGATION_Update(float dt)
{
    // ============================================================
    // 1. GİRDİLERİ AL
    // ============================================================
    // İvme Kaynağı: MPU9250 X Ekseni (Podun gidiş yönü)
    float input_accel = shared_data.sensors.mpu.accel_x_mss;
    
    // Hız Kaynağı: Encoder (Filtrelenmiş veya Ham kullanılabilir)
    float measured_velocity = shared_data.sensors.encoder.velocity_mps;
    // ============================================================
    // 2. TAHMİN (PREDICTION- TIME UPDATE) - Fizik Modeli
    // ============================================================
    // İvme hatasını (bias) çıkararak saf ivmeyi bul
    float accel_net = input_accel - nav_state.bias_accel;
    
    // Konum Tahmini: x = x + v*t + 0.5*a*t^2
    nav_state.position_m += (nav_state.velocity_mps * dt) + (0.5f * accel_net * dt * dt);

    // Hız Tahmini: v = v + a*t
    nav_state.velocity_mps += accel_net * dt;
    
    // Hata matrisini (P) büyüt (Zaman geçtikçe belirsizlik artar)
    nav_state.P[0][0] += KF_Q_ACCEL * dt; 
    nav_state.P[1][1] += KF_Q_ACCEL * dt;

    // ============================================================
    // 3. DÜZELTME (CORRECTION - MEASUREMENT UPDATE) - Encoder ile
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
    
    // KOVARYANS GÜNCELLEME (Ölçüm geldiği için belirsizlik azaldı)
    nav_state.P[1][1] = (1.0f - K_gain) * nav_state.P[1][1];


   // ============================================================
    // 4. SONUÇLARI YAZ (Shared Data'ya)
    // ============================================================
    
    // Artık "encoder hızı" değil, "Füzyon Hızı"nı sisteme sunuyoruz.
    // Fren ve Motor algoritmaları ARTIK BU VERİYİ KULLANMALI!
    
    shared_data.sensors.nav.position_m     = nav_state.position_m;
    shared_data.sensors.nav.velocity_mps   = nav_state.velocity_mps;
    shared_data.sensors.nav.accel_bias_mss = nav_state.bias_accel;
}
*/
