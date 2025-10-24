#include "hal_uart.h"

extern UART_HandleTypeDef huart2; // CubeMX’te UART2 konfigüre edildiğinde oluşur

void hal_uart_send(uint8_t *data, uint16_t size)
{
    HAL_UART_Transmit(&huart2, data, size, HAL_MAX_DELAY);
}

void hal_uart_receive(uint8_t *data, uint16_t size)
{
    HAL_UART_Receive(&huart2, data, size, HAL_MAX_DELAY);
}

