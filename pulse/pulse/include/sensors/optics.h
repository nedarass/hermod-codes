// Core/Inc/optics.h
#ifndef INC_OPTICS_H_
#define INC_OPTICS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "shared_data.h"

typedef enum {
    OMRON_NO_OBSTACLE = 0,
    OMRON_OBSTACLE_DETECTED = 1
} Omron_State_t;

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

// Callback: main.c içindeki HAL_GPIO_EXTI_Callback'ten çağrılmalı
void OPTICS_EXTI_Callback(uint16_t GPIO_Pin);

// Control.c Arayüzü
uint8_t OPTICS_IsEmergencyObstacleDetected(void);
void OPTICS_ClearEmergencyFlag(void);
Omron_State_t OPTICS_GetSensorState(uint8_t sensor_id);

#ifdef __cplusplus
}
#endif

#endif /* INC_OPTICS_H_ */
/*HAL GPIO EXTI: Omron veya Optik sensörler için STM32 Harici Kesme (EXTI)
kullanılmalıdır. Bu, Omron algılandığında anında tepki vermenin doğru yoludur.*/

