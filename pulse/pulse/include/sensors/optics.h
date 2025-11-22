// Core/Inc/optics.h
#ifndef INC_OPTICS_H_
#define INC_OPTICS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "shared_data.h"

// Omron Sensör Pin Tanımları - CubeMX'te bu pinleri yapılandır!
// bunları pin de yapabilrim ? 
#define OMRON_SENSOR_1_PIN       GPIO_PIN_0
#define OMRON_SENSOR_1_PORT      GPIOA
#define OMRON_SENSOR_2_PIN       GPIO_PIN_1  
#define OMRON_SENSOR_2_PORT      GPIOA

// Omron Sensör Durumları
typedef enum {
    OMRON_OBSTACLE_DETECTED = 0,    // Engel var (LOW)
    OMRON_NO_OBSTACLE = 1           // Engel yok (HIGH)
} Omron_State_t;

// Omron Sensör Yapısı
typedef struct {
    GPIO_TypeDef* GPIO_Port;
    uint16_t GPIO_Pin;
    Omron_State_t last_state;
    uint32_t detection_timestamp;
    uint8_t sensor_id;
} Omron_Sensor_t;

// Fonksiyon Prototipleri
void OPTICS_Init(void);
void OPTICS_Update(void);
Omron_State_t OPTICS_GetSensorState(uint8_t sensor_id);
uint32_t OPTICS_GetDetectionTime(uint8_t sensor_id);
uint8_t OPTICS_IsAnyObstacleDetected(void);

// EXTI Callback - main.c'deki HAL_GPIO_EXTI_Callback'tan çağrılacak
void OPTICS_EXTI_Callback(uint16_t GPIO_Pin);

#ifdef __cplusplus
}
#endif

#endif /* INC_OPTICS_H_ */
/*HAL GPIO EXTI: Omron veya Optik sensörler için STM32 Harici Kesme (EXTI)
kullanılmalıdır. Bu, Omron algılandığında anında tepki vermenin doğru yoludur.*/

