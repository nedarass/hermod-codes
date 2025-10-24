#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include "stm32f4xx_hal.h"
#include "board_config.h"

#ifdef __cplusplus
extern "C" {
#endif

void hal_gpio_init(GPIO_TypeDef *port, uint16_t pin, uint32_t mode);
void hal_gpio_write(GPIO_TypeDef *port, uint16_t pin, uint8_t value);
uint8_t hal_gpio_read(GPIO_TypeDef *port, uint16_t pin);

#ifdef __cplusplus
}
#endif

#endif // HAL_GPIO_H

