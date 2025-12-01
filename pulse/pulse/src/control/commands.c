#include "commands.h"
#include "actuators/brakes.h"
#include "actuators/vesc6.h"
#include "actuators/inverter_rs485.h"
#include "actuators/power_cut.h"
#include "shared_data.h"
#include <string.h>
#include <stdio.h>

// Harici değişkenler (main.c'de tanimlanan)
extern UART_HandleTypeDef huart1;

// --- AYARLAR ---
#define WHEEL_RADIUS_M   0.1f         // Tekerlek Yarıçapı (10 cm)
#define MAX_SPEED_LIMIT  80.0f        // m/s (Güvenlik Limiti)

// -------------------------------------------------------------------
// --- SİMULASYON AKTÜATÖR FONKSİYONLARI ---
// -------------------------------------------------------------------

void BRAKE_SetForce(uint8_t force)
{
    // Gerçekte: Fren solenoidi/motoru kontrolü (PWM, Dijital pin vb.)
     // Güvenlik kontrolü
    if (force > 100) force = 100; // Max %100 force
    
    // Gerçek brake kontrolü
    if (force > 0) {
        BRAKES_SetState(BRAKE_ENGAGED);
    } else {
        BRAKES_SetState(BRAKE_RELEASED);
    }
    
    // Durumu kaydet
   shared_data.actuators.brake_state = (force > 0) ? BRAKE_ENGAGED : BRAKE_RELEASED;
printf("BRAKE: Force=%u%%, State=%s\r\n", force, 
       (force > 0) ? "ENGAGED" : "RELEASED");
}

void MOTOR_SetTargetSpeed(float speed_mps)
{
    // Hız limit kontrolü
    if (speed_mps < 0) speed_mps = 0;
    if (speed_mps > 50.0f) speed_mps = MAX_SPEED_LIMIT; // Max 50 m/s (~180 km/h)
    
    // RPM hesaplama (tekerlek çapına göre)
    float wheel_rpm = (speed_mps * 60.0f) / (2.0f * 3.14159f * WHEEL_RADIUS_M); // 0.1m yarıçap varsayımı

    shared_data.actuators.target_rpm = (int32_t)wheel_rpm;
    
    // Motor sürücüsüne komut gönder
    if (!VESC_SetRPM((int32_t)wheel_rpm)) 
    {
        shared_data.system.error_flags |= ERR_COMM_TIMEOUT;
    }
}

void SYSTEM_EmergencyPowerCut(void)
{
     //printf("🚨 KRITIK KOMUT: ACIL GUC KESME! Sistemi kapatiliyor...\r\n");
    
    // 1. GERÇEK GÜÇ KESME - power_cut.h'daki fonksiyonu çağır
    POWERCUT_TriggerEmergency();
    
    // 2. FRENLERİ KİLİTLE (Güvenlik)
    BRAKES_EmergencyEngage();
    
    // 3. Sistem Durumunu Güncelle
    shared_data.system.emergency_mode = true;
    shared_data.system.error_flags |= ERR_POWER_TRIP;
    
    // Motoru da sustur
    VESC_SetRPM(0);
}

// -------------------------------------------------------------------
// --- ANA KOMUT İŞLEYİCİ (COMMUNICATION.C'DEN ÇAĞRILIR) ---
// Communication modülünden gelen ham paketi işler ve yönlendirir.
// -------------------------------------------------------------------

void COMMAND_ProcessIncoming(uint8_t id, uint8_t type, uint8_t *payload, uint16_t len)
{
    switch (id)
    {
        // --- SENARYO A: FREN KOMUTU GELDİ ---
        case CMD_BRAKE_ACTUATE:
        {
            if (type == TYPE_U8 && len == 1) {
                uint8_t brake_force = payload[0];
                BRAKE_SetForce(brake_force);
            } else {
                 printf("HATA: CMD_BRAKE_ACTUATE paketi hatali.\r\n");
            }
            break;
        }
        // --- SENARYO B: HIZ KOMUTU GELDİ ---
        case CMD_SET_TARGET_SPEED:
        {
            if (type == TYPE_F32 && len == sizeof(float)) {
                float target_speed;
                // C++ std::memcpy yerine C standart memcpy kullanılır
                memcpy(&target_speed, payload, sizeof(float));
                MOTOR_SetTargetSpeed(target_speed);
            } else {
                printf("HATA: CMD_SET_TARGET_SPEED paketi hatali.\r\n");
            }
            break;
        }
        // --- SENARYO C: ACİL DURDURMA GELDİ ---
        case CMD_POWER_CUT_OFF:
        {
            SYSTEM_EmergencyPowerCut();
            break;
        }
        // --- SENARYO D: PING ---
        case CMD_PING_REQUEST:
        {
            // Ping isteği iletişim katmanında (communication.c) cevaplanır.
            printf("PING ISTEDI.\r\n");
            break;
        }
        default:
            printf("BILINMEYEN KOMUT ID: 0x%02X\r\n", id);
            break;
    }
}
