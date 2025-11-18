// Core/Inc/ntc_sensor.h

#ifndef INC_NTC_SENSOR_H_
#define INC_NTC_SENSOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h" // HAL tipleri için (ADC_HandleTypeDef)

// NTC Sensor Fonksiyonları (C++'da tanımlanacak)
void NTC_Init(void);
void NTC_Update(ADC_HandleTypeDef *hadc); // ADC Handle'ı argüman olarak alır

#ifdef __cplusplus
}
#endif

#endif /* INC_NTC_SENSOR_H_ */
