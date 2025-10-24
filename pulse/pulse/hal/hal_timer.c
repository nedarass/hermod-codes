#include "hal_timer.h"

void hal_timer_start(TIM_HandleTypeDef *htim)
{
    HAL_TIM_Base_Start(htim);
}

void hal_timer_stop(TIM_HandleTypeDef *htim)
{
    HAL_TIM_Base_Stop(htim);
}

void hal_pwm_set_duty(TIM_HandleTypeDef *htim, uint32_t channel, uint16_t duty)
{
    __HAL_TIM_SET_COMPARE(htim, channel, duty);
}

