// Core/Src/optics.c
#include "optics.h"
#include "shared_data.h"
#include <stdio.h>

// Global sistem durumu
extern System_State_t g_system_state;

// Omron Sensörleri
static Omron_Sensor_t omron_sensors[2] = {
    {OMRON_SENSOR_1_PORT, OMRON_SENSOR_1_PIN, OMRON_NO_OBSTACLE, 0, 1},
    {OMRON_SENSOR_2_PORT, OMRON_SENSOR_2_PIN, OMRON_NO_OBSTACLE, 0, 2}
};

// Acil durum flag'i - SADECE control.c bunu okuyup karar verecek
static uint8_t emergency_obstacle_detected = 0;

// -------------------------------------------------------------------
// --- BAŞLATMA FONKSİYONU ---
// -------------------------------------------------------------------
void OPTICS_Init(void)
{
    printf("OPTICS: Omron sensörleri başlatılıyor...\r\n");
    
    for (int i = 0; i < 2; i++) {
        omron_sensors[i].last_state = OMRON_NO_OBSTACLE;
        omron_sensors[i].detection_timestamp = 0;
    }
    
    emergency_obstacle_detected = 0;
    
    printf("OPTICS: Omron sensörleri hazır (EXTI aktif)\r\n");
}

// -------------------------------------------------------------------
// --- EXTI KESME CALLBACK'I - SADECE VERİYİ KAYDET ---
// -------------------------------------------------------------------
void OPTICS_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint32_t current_time = HAL_GetTick();
    
    for (int i = 0; i < 2; i++) {
        if (omron_sensors[i].GPIO_Pin == GPIO_Pin) {
            GPIO_PinState pin_state = HAL_GPIO_ReadPin(omron_sensors[i].GPIO_Port, omron_sensors[i].GPIO_Pin);
            Omron_State_t new_state = (pin_state == GPIO_PIN_SET) ? OMRON_NO_OBSTACLE : OMRON_OBSTACLE_DETECTED;
            
            if (new_state != omron_sensors[i].last_state) {
                omron_sensors[i].last_state = new_state;
                
                if (new_state == OMRON_OBSTACLE_DETECTED) {
                    omron_sensors[i].detection_timestamp = current_time;
                    emergency_obstacle_detected = 1; // ✅ SADECE FLAG SET ET!
                    
                    printf("🚨 OMRON %d EXTI: Engel tespit edildi (Flag set)\r\n", omron_sensors[i].sensor_id);
                    
                    // ❌❌❌ BURADA KESİNLİKLE ACTUATOR ÇAĞIRMA! ❌❌❌
                    // CONTROL.c bunu okuyup gerekli kararı verecek
                    
                } else {
                    printf("✅ OMRON %d EXTI: Engel kalktı\r\n", omron_sensors[i].sensor_id);
                }
            }
            break;
        }
    }
}

// -------------------------------------------------------------------
// --- OMRON DURUM GÜNCELLEME ---  
// -------------------------------------------------------------------
void OPTICS_Update(void)
{
    // Periyodik durum kontrolü (polling backup)
    for (int i = 0; i < 2; i++) {
        GPIO_PinState pin_state = HAL_GPIO_ReadPin(omron_sensors[i].GPIO_Port, omron_sensors[i].GPIO_Pin);
        Omron_State_t current_state = (pin_state == GPIO_PIN_SET) ? OMRON_NO_OBSTACLE : OMRON_OBSTACLE_DETECTED;
        
        if (current_state != omron_sensors[i].last_state) {
            omron_sensors[i].last_state = current_state;
            
            if (current_state == OMRON_OBSTACLE_DETECTED) {
                omron_sensors[i].detection_timestamp = HAL_GetTick();
                emergency_obstacle_detected = 1; // ✅ FLAG SET ET
                printf("🚨 OMRON %d POLLING: Engel tespit edildi\r\n", omron_sensors[i].sensor_id);
            }
        }
    }
}

// -------------------------------------------------------------------
// --- DURUM SORGULAMA FONKSİYONLARI ---
// -------------------------------------------------------------------

// ✅ Control.c bunları kullanacak
uint8_t OPTICS_IsEmergencyObstacleDetected(void)
{
    return emergency_obstacle_detected;
}

// ✅ Acil durumu resetle - control.c kullanacak
void OPTICS_ClearEmergencyFlag(void)
{
    emergency_obstacle_detected = 0;
}

Omron_State_t OPTICS_GetSensorState(uint8_t sensor_id)
{
    for (int i = 0; i < 2; i++) {
        if (omron_sensors[i].sensor_id == sensor_id) {
            return omron_sensors[i].last_state;
        }
    }
    return OMRON_NO_OBSTACLE;
}

uint32_t OPTICS_GetDetectionTime(uint8_t sensor_id)
{
    for (int i = 0; i < 2; i++) {
        if (omron_sensors[i].sensor_id == sensor_id) {
            return omron_sensors[i].detection_timestamp;
        }
    }
    return 0;
}

uint8_t OPTICS_IsAnyObstacleDetected(void)
{
    for (int i = 0; i < 2; i++) {
        if (omron_sensors[i].last_state == OMRON_OBSTACLE_DETECTED) {
            return 1;
        }
    }
    return 0;
}
