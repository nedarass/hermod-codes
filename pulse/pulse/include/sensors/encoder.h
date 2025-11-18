// Core/Inc/encoder.h

#ifndef INC_ENCODER_H_
#define INC_ENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h" // TIM_HandleTypeDef yapısı için gerekli

// --- FONKSİYON PROTOTİPLERİ ---

/**
 * @brief Encoder için zamanlayıcıyı (Timer) başlatır.
 * @param htim: CubeMX'te Encoder Modunda ayarlanan Timer Handle (örn: &htim3)
 */
void ENCODER_Init(TIM_HandleTypeDef *htim);

/**
 * @brief Timer sayacını okur, hızı ve konumu hesaplayıp shared_data'ya yazar.
 * Bu fonksiyon control.c içindeki döngüden (örn: 10ms'de bir) çağrılmalıdır.
 * @param htim: Kullanılan Timer Handle
 */
void ENCODER_Update(TIM_HandleTypeDef *htim);

#ifdef __cplusplus
}
#endif

#endif /* INC_ENCODER_H_ */


/* HAL TIM Encoder Modu: PIO'nun yerine, STM32'nin donanımsal Timer Encoder
Modu kullanılmalıdır. Bu, CPU yükü olmadan pozisyon sayımı
için en verimli yöntemdir.*/
