#ifndef HAL_UART_H
#define HAL_UART_H

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

void hal_uart_send(uint8_t *data, uint16_t size);
void hal_uart_receive(uint8_t *data, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif // HAL_UART_H

