// Core/Inc/optics.h
#ifndef INC_OPTICS_H_
#define INC_OPTICS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "shared_data.h"

// ... diğer tanımlamalar aynı

// Fonksiyon Prototipleri
void OPTICS_Init(void);
void OPTICS_Update(void);
void OPTICS_EXTI_Callback(uint16_t GPIO_Pin);

// ✅ YENİ EKLENDİ - Control.c bunları kullanacak
uint8_t OPTICS_IsEmergencyObstacleDetected(void);
void OPTICS_ClearEmergencyFlag(void);
Omron_State_t OPTICS_GetSensorState(uint8_t sensor_id);
uint32_t OPTICS_GetDetectionTime(uint8_t sensor_id);
uint8_t OPTICS_IsAnyObstacleDetected(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_OPTICS_H_ */
/*HAL GPIO EXTI: Omron veya Optik sensörler için STM32 Harici Kesme (EXTI)
kullanılmalıdır. Bu, Omron algılandığında anında tepki vermenin doğru yoludur.*/

