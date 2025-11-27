// pulse/pulse/src/sensors/optics.c
#include "optics.h"
#include "shared_data.h"
#include <stdio.h>

// --- PIN TANIMLARI (Güvenli Tanımlama) ---
// Eğer CubeMX etiketleri yoksa, varsayılan pinleri tanımla
#ifndef OPTIC_1_Pin
    #define OPTIC_1_GPIO_Port GPIOA
    #define OPTIC_1_Pin       GPIO_PIN_0
#endif

#ifndef OPTIC_2_Pin
    #define OPTIC_2_GPIO_Port GPIOA
    #define OPTIC_2_Pin       GPIO_PIN_1
#endif

// Sensör Dizisi (Değerler Init fonksiyonunda atanacak)
static Omron_Sensor_t omron_sensors[2];

// Acil durum flag'i
static uint8_t emergency_obstacle_detected = 0;

// -------------------------------------------------------------------
// --- BAŞLATMA FONKSİYONU ---
// -------------------------------------------------------------------
void OPTICS_Init(void)
{
    // 1. Sensör Pinlerini ve Varsayılan Değerleri Ata (DÜZELTME BURADA YAPILDI)
    omron_sensors[0].GPIO_Port = OPTIC_1_GPIO_Port;
    omron_sensors[0].GPIO_Pin  = OPTIC_1_Pin;
    omron_sensors[0].sensor_id = 1;

    omron_sensors[1].GPIO_Port = OPTIC_2_GPIO_Port;
    omron_sensors[1].GPIO_Pin  = OPTIC_2_Pin;
    omron_sensors[1].sensor_id = 2;

    // 2. Durumları Sıfırla
    for (int i = 0; i < 2; i++) {
        omron_sensors[i].last_state = OMRON_NO_OBSTACLE;
        omron_sensors[i].detection_timestamp = 0;
    }
    
    emergency_obstacle_detected = 0;
    
    // Shared Data Temizliği
    shared_data.sensors.optics.obstacle_detected = 0;
    shared_data.sensors.optics.raw_sensor_1 = 0;
    shared_data.sensors.optics.raw_sensor_2 = 0;
    
    printf("Optik Sensorler Baslatildi.\r\n");
}

// -------------------------------------------------------------------
// --- EXTI KESME CALLBACK'I ---
// -------------------------------------------------------------------
void OPTICS_EXTI_Callback(uint16_t GPIO_Pin)
{
    for (int i = 0; i < 2; i++) 
    {
        if (omron_sensors[i].GPIO_Pin == GPIO_Pin) 
        {
            // Pini Oku
            GPIO_PinState pin_state = HAL_GPIO_ReadPin(omron_sensors[i].GPIO_Port, omron_sensors[i].GPIO_Pin);
            
            // Durumu Belirle (1: Engel Var, 0: Yok)
            Omron_State_t new_state = (pin_state == GPIO_PIN_SET) ? OMRON_OBSTACLE_DETECTED : OMRON_NO_OBSTACLE;
            
            // Durum değiştiyse kaydet
            if (new_state != omron_sensors[i].last_state) 
            {
                omron_sensors[i].last_state = new_state;
                
                if (new_state == OMRON_OBSTACLE_DETECTED) 
                {
                    omron_sensors[i].detection_timestamp = HAL_GetTick();
                    emergency_obstacle_detected = 1;
                    
                    // Shared Data Güncelle (Anında)
                    shared_data.sensors.optics.obstacle_detected = 1;
                }
            }
            
            // Shared Data Raw Güncelle
            if (i == 0) shared_data.sensors.optics.raw_sensor_1 = new_state;
            else        shared_data.sensors.optics.raw_sensor_2 = new_state;
            
            break; // İlgili pini bulduk, çık
        }
    }
}

// -------------------------------------------------------------------
// --- OMRON DURUM GÜNCELLEME (Polling) ---  
// -------------------------------------------------------------------
void OPTICS_Update(void)
{
    // Interrupt kaçarsa diye periyodik kontrol (DÜZELTME: İçi dolduruldu)
    for (int i = 0; i < 2; i++) 
    {
        GPIO_PinState pin_state = HAL_GPIO_ReadPin(omron_sensors[i].GPIO_Port, omron_sensors[i].GPIO_Pin);
        Omron_State_t current_state = (pin_state == GPIO_PIN_SET) ? OMRON_OBSTACLE_DETECTED : OMRON_NO_OBSTACLE;
        
        // Son durumu güncelle
        omron_sensors[i].last_state = current_state;
        
        // Eğer engel varsa bayrağı dik
        if (current_state == OMRON_OBSTACLE_DETECTED) {
             emergency_obstacle_detected = 1;
             shared_data.sensors.optics.obstacle_detected = 1;
        }
        
        // Shared Data güncelle
        if (i == 0) shared_data.sensors.optics.raw_sensor_1 = current_state;
        else        shared_data.sensors.optics.raw_sensor_2 = current_state;
    }
}

// -------------------------------------------------------------------
// --- DURUM SORGULAMA FONKSİYONLARI ---
// -------------------------------------------------------------------

uint8_t OPTICS_IsEmergencyObstacleDetected(void)
{
    return emergency_obstacle_detected;
}

void OPTICS_ClearEmergencyFlag(void)
{
    emergency_obstacle_detected = 0;
    shared_data.sensors.optics.obstacle_detected = 0;
}

Omron_State_t OPTICS_GetSensorState(uint8_t sensor_id)
{
    if (sensor_id == 1) return omron_sensors[0].last_state;
    if (sensor_id == 2) return omron_sensors[1].last_state;
    return OMRON_NO_OBSTACLE;
}
