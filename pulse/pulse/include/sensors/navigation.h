// Core/Inc/navigation.h

#ifndef INC_NAVIGATION_H_
#define INC_NAVIGATION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h" // STM32 HAL tipleri için

// --- FONKSİYON PROTOTİPLERİ ---

/**
 * @brief Navigasyon sistemini ve Kalman filtresi değişkenlerini sıfırlar.
 * Control_Init içinde çağrılır.
 */
void NAVIGATION_Init(void);

/**
 * @brief Sensör Füzyonu Algoritmasını çalıştırır.
 * shared_data'dan MPU ve Encoder verilerini alır, işler ve
 * filtrelenmiş sonuçları tekrar shared_data'ya yazar.
 *
 * @param dt: Geçen süre (saniye cinsinden). Örn: 0.01f (10ms)
 */
void NAVIGATION_Update(float dt);

#ifdef __cplusplus
}
#endif

#endif /* INC_NAVIGATION_H_ */
