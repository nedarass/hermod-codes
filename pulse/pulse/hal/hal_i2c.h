#ifndef HAL_I2C_H
#define HAL_I2C_H

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

void hal_i2c_init(void);
HAL_StatusTypeDef hal_i2c_write(uint8_t dev_addr, uint8_t *data, uint16_t size);
HAL_StatusTypeDef hal_i2c_read(uint8_t dev_addr, uint8_t *data, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif // HAL_I2C_H

