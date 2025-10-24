#ifndef HAL_TIMER_H
#define HAL_TIMER_H

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

void hal_timer_start(TIM_HandleTypeDef *htim);
void hal_timer_stop(TIM_HandleTypeDef *htim);
void hal_pwm_set_duty(TIM_HandleTypeDef *htim, uint32_t channel, uint16_t duty);

#ifdef __cplusplus
}
#endif

#endif // HAL_TIMER_H

