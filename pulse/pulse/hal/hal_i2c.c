#include "hal_i2c.h"

extern I2C_HandleTypeDef hi2c1; // CubeIDE'de I2C1 konfigüre edildiğinde otomatik oluşur

void hal_i2c_init(void)
{
    // I2C1 CubeMX üzerinden init edilir.
    // Burada özel ek ayar yapmak gerekirse eklenebilir.
}

HAL_StatusTypeDef hal_i2c_write(uint8_t dev_addr, uint8_t *data, uint16_t size)
{
    return HAL_I2C_Master_Transmit(&hi2c1, dev_addr << 1, data, size, HAL_MAX_DELAY);
}

HAL_StatusTypeDef hal_i2c_read(uint8_t dev_addr, uint8_t *data, uint16_t size)
{
    return HAL_I2C_Master_Receive(&hi2c1, dev_addr << 1, data, size, HAL_MAX_DELAY);
}

