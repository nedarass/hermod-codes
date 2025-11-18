// Core/Inc/ntc_sensor.h

#ifndef INC_NTC_SENSOR_H_
#define INC_NTC_SENSOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h" // HAL tipleri için (ADC_HandleTypeDef)

// NTC Sensor Fonksiyonları 
// Başlatma fonksiyonu (şu an boş ama gelecekte kalibrasyon için gerekebilir)
void NTC_Init(void);
// Güncelleme fonksiyonu: Ana kontrol döngüsünden çağrılır.
// Parametre olarak ADC biriminin adresini (Handle) alır.
void NTC_Update(ADC_HandleTypeDef *hadc); 

#ifdef __cplusplus
}
#endif

#endif /* INC_NTC_SENSOR_H_ */
