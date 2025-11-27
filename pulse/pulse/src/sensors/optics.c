#include "optics.h"
#include "shared_data.h"
#include <stdio.h>

// --- PIN TANIMLARI (CubeMX Etiketleri) ---
// CubeMX'te sensör pinlerine "OPTIC_1" ve "OPTIC_2" etiketi verdiğini varsayıyorum.
// Eğer vermediysen main.h içine bakıp doğru pinleri buraya yazmalısın.

#ifndef OPTIC_1_Pin
    // Eğer CubeMX'te etiket verilmediyse manuel tanımla (Örn: PA0 ve PA1)
    #define OPTIC_1_GPIO_Port GPIOA
    #define OPTIC_1_Pin       GPIO_PIN_0
    #define OPTIC_2_GPIO_Port GPIOA
    #define OPTIC_2_Pin       GPIO_PIN_1
#endif

// Omron Sensörleri - AYNI pinleri kullan:
static Omron_Sensor_t omron_sensors[2] = {
    {OPTIC_1_GPIO_Port, OPTIC_1_Pin, OMRON_NO_OBSTACLE, 0, 1}, 
    {OPTIC_2_GPIO_Port, OPTIC_2_Pin, OMRON_NO_OBSTACLE, 0, 2}   
};

// Acil durum flag'i - SADECE control.c bunu okuyup karar verecek
static uint8_t emergency_obstacle_detected = 0;

// -------------------------------------------------------------------
// --- BAŞLATMA FONKSİYONU ---
// -------------------------------------------------------------------
void OPTICS_Init(void)
{
    for (int i = 0; i < 2; i++) {
        omron_sensors[i].last_state = OMRON_NO_OBSTACLE;
        omron_sensors[i].detection_timestamp = 0;
    }
    
    emergency_obstacle_detected = 0;
    shared_data.sensors.optics.obstacle_detected = 0;
    shared_data.sensors.optics.raw_sensor_1 = 0;
    shared_data.sensors.optics.raw_sensor_2 = 0;
}

// -------------------------------------------------------------------
// --- EXTI KESME CALLBACK'I - SADECE VERİYİ KAYDET ---
// -------------------------------------------------------------------
void OPTICS_EXTI_Callback(uint16_t GPIO_Pin)
{
    for (int i = 0; i < 2; i++) 
    {
        if (omron_sensors[i].GPIO_Pin == GPIO_Pin) 
        {
            // Pini Oku (Active Low veya High olabilir, sensöre göre değişir)
            // Varsayım: Engel varken HIGH (1), yokken LOW (0)
            GPIO_PinState pin_state = HAL_GPIO_ReadPin(omron_sensors[i].GPIO_Port, omron_sensors[i].GPIO_Pin);
            
            Omron_State_t new_state = (pin_state == GPIO_PIN_SET) ? OMRON_OBSTACLE_DETECTED : OMRON_NO_OBSTACLE;
            
            // Durum değiştiyse işlem yap
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
            
            break; // İlgili pini bulduk, döngüden çık
        }
    }
}

// -------------------------------------------------------------------
// --- OMRON DURUM GÜNCELLEME ---  
// -------------------------------------------------------------------
void OPTICS_Update(void)
{
    // Interrupt kaçarsa diye periyodik kontrol
    for (int i = 0; i < 2; i++) 
    {
        // ... (EXTI ile aynı mantık, kodu tekrar etmemek için özeti yazıyorum)
        // Pini oku, durum değiştiyse shared_data'yı güncelle.
        // Kodun EXTI kısmındaki mantığın aynısı buraya uygulanabilir.
        
        GPIO_PinState pin_state = HAL_GPIO_ReadPin(omron_sensors[i].GPIO_Port, omron_sensors[i].GPIO_Pin);
        Omron_State_t current_state = (pin_state == GPIO_PIN_SET) ? OMRON_OBSTACLE_DETECTED : OMRON_NO_OBSTACLE;
        
        if (current_state == OMRON_OBSTACLE_DETECTED) {
             emergency_obstacle_detected = 1;
             shared_data.sensors.optics.obstacle_detected = 1;
        }
        
        if (i == 0) shared_data.sensors.optics.raw_sensor_1 = current_state;
        else        shared_data.sensors.optics.raw_sensor_2 = current_state;
    }
}

// -------------------------------------------------------------------
// --- DURUM SORGULAMA FONKSİYONLARI ---
// -------------------------------------------------------------------

// --- CONTROL ARAYÜZÜ ---
uint8_t OPTICS_IsEmergencyObstacleDetected(void)
{
    return emergency_obstacle_detected;
}

void OPTICS_ClearEmergencyFlag(void)
{
    emergency_obstacle_detected = 0;
    shared_data.sensors.optics.obstacle_detected = 0;
}
