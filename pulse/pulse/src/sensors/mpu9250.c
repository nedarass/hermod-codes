// Core/Src/mpu9250.c
#include "mpu9250.h"
#include <stdio.h>
#include <string.h>

// Global sistem durumu
extern System_State_t g_system_state;

// MPU9250 veri buffer'ı
static MPU9250_Data_t mpu9250_data;
static uint8_t mpu9250_data_ready = 0;
static I2C_HandleTypeDef* mpu9250_hi2c = NULL;

// DMA için buffer
static uint8_t mpu9250_rx_buffer[14];

// -------------------------------------------------------------------
// --- DÜŞÜK SEVİYE I2C FONKSİYONLARI ---
// -------------------------------------------------------------------

// I2C'ye 1 byte yaz
static HAL_StatusTypeDef MPU9250_WriteByte(uint8_t reg, uint8_t data)
{
    return HAL_I2C_Mem_Write(mpu9250_hi2c, MPU9250_I2C_ADDR, reg, 
                           I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
}

// I2C'den multiple byte oku (DMA ile)
static HAL_StatusTypeDef MPU9250_ReadBytes_DMA(uint8_t reg, uint8_t *data, uint16_t size)
{
    return HAL_I2C_Mem_Read_DMA(mpu9250_hi2c, MPU9250_I2C_ADDR, reg,
                              I2C_MEMADD_SIZE_8BIT, data, size);
}

// I2C'den multiple byte oku (Blocking)
static HAL_StatusTypeDef MPU9250_ReadBytes(uint8_t reg, uint8_t *data, uint16_t size)
{
    return HAL_I2C_Mem_Read(mpu9250_hi2c, MPU9250_I2C_ADDR, reg,
                          I2C_MEMADD_SIZE_8BIT, data, size, 100);
}

// -------------------------------------------------------------------
// --- MPU9250 BAŞLATMA FONKSİYONU ---
// -------------------------------------------------------------------
HAL_StatusTypeDef MPU9250_Init(I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef status;
    uint8_t whoami;
    
    if (hi2c == NULL) {
        return HAL_ERROR;
    }
    
    mpu9250_hi2c = hi2c;
    
    printf("MPU9250: Baslatiliyor...\r\n");
    
    // 1. WHO_AM_I kontrolü - Cihaz var mı?
    status = MPU9250_ReadBytes(MPU9250_WHO_AM_I, &whoami, 1);
    if (status != HAL_OK) {
        printf("MPU9250: I2C hatasi! Cihaz yanit vermiyor.\r\n");
        return status;
    }
    
    if (whoami != MPU9250_WHO_AM_I_VALUE) {
        printf("MPU9250: WHO_AM_I hatasi! Beklenen: 0x%02X, Alinan: 0x%02X\r\n", 
               MPU9250_WHO_AM_I_VALUE, whoami);
        return HAL_ERROR;
    }
    
    printf("MPU9250: Cihaz bulundu (WHO_AM_I: 0x%02X)\r\n", whoami);
    
    // 2. Reset ve ayarlar
    // Power Management 1 - Reset + Clock Source
    status = MPU9250_WriteByte(MPU9250_PWR_MGMT_1, 0x80); // Device Reset
    if (status != HAL_OK) return status;
    HAL_Delay(100);
    
    // Power Management 1 - Clock Source = PLL with X axis gyro reference
    status = MPU9250_WriteByte(MPU9250_PWR_MGMT_1, 0x01);
    if (status != HAL_OK) return status;
    
    // Configuration - DLPF_CFG = 3 (44Hz BW)
    status = MPU9250_WriteByte(MPU9250_CONFIG, 0x03);
    if (status != HAL_OK) return status;
    
    // Gyro Configuration - FS_SEL = 0 (±250dps)
    status = MPU9250_WriteByte(MPU9250_GYRO_CONFIG, 0x00);
    if (status != HAL_OK) return status;
    
    // Accelerometer Configuration - AFS_SEL = 0 (±2g)
    status = MPU9250_WriteByte(MPU9250_ACCEL_CONFIG, 0x00);
    if (status != HAL_OK) return status;
    
    // Accelerometer Configuration 2 - A_DLPF_CFG = 3 (44Hz BW)
    status = MPU9250_WriteByte(MPU9250_ACCEL_CONFIG2, 0x03);
    if (status != HAL_OK) return status;
    
    // Sample Rate Divider - 1kHz/(1+7) = 125Hz
    status = MPU9250_WriteByte(MPU9250_SMPLRT_DIV, 0x07);
    if (status != HAL_OK) return status;
    
    // Interrupt Enable - Data Ready Enable
    status = MPU9250_WriteByte(MPU9250_INT_ENABLE, 0x01);
    if (status != HAL_OK) return status;
    
    // User Control - I2C Master Enable disable, FIFO disable
    status = MPU9250_WriteByte(MPU9250_USER_CTRL, 0x00);
    if (status != HAL_OK) return status;
    
    // Veri yapısını sıfırla
    memset(&mpu9250_data, 0, sizeof(MPU9250_Data_t));
    mpu9250_data_ready = 0;
    
    printf("MPU9250: Basarili sekilde baslatildi\r\n");
    return HAL_OK;
}

// -------------------------------------------------------------------
// --- MPU9250 OKUMA TETİKLEME (DMA) ---
// -------------------------------------------------------------------
HAL_StatusTypeDef MPU9250_Trigger_Read(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == NULL || mpu9250_hi2c == NULL) {
        return HAL_ERROR;
    }
    
    // DMA ile 14 byte oku (ACCEL_XOUT_H'den başlayarak)
    return MPU9250_ReadBytes_DMA(MPU9250_ACCEL_XOUT_H, mpu9250_rx_buffer, 14);
}

// -------------------------------------------------------------------
// --- DMA OKUMA TAMAMLANDI CALLBACK ---
// -------------------------------------------------------------------
void MPU9250_Read_DMA_Complete_Callback(void)
{
    // DMA transferi tamamlandı, veriyi işle
    
    // Raw verileri 16-bit değerlere çevir
    mpu9250_data.accel_x = (int16_t)((mpu9250_rx_buffer[0] << 8) | mpu9250_rx_buffer[1]);
    mpu9250_data.accel_y = (int16_t)((mpu9250_rx_buffer[2] << 8) | mpu9250_rx_buffer[3]);
    mpu9250_data.accel_z = (int16_t)((mpu9250_rx_buffer[4] << 8) | mpu9250_rx_buffer[5]);
    mpu9250_data.temp    = (int16_t)((mpu9250_rx_buffer[6] << 8) | mpu9250_rx_buffer[7]);
    mpu9250_data.gyro_x  = (int16_t)((mpu9250_rx_buffer[8] << 8) | mpu9250_rx_buffer[9]);
    mpu9250_data.gyro_y  = (int16_t)((mpu9250_rx_buffer[10] << 8) | mpu9250_rx_buffer[11]);
    mpu9250_data.gyro_z  = (int16_t)((mpu9250_rx_buffer[12] << 8) | mpu9250_rx_buffer[13]);
    
    mpu9250_data.timestamp = HAL_GetTick();
    mpu9250_data_ready = 1;
    
    // Global sisteme yaz
    g_system_state.acceleration_mss = (float)mpu9250_data.accel_x / MPU9250_ACCEL_SCALE;
    
    // Debug
    // printf("MPU: ax=%d, ay=%d, az=%d, gx=%d, gy=%d, gz=%d\r\n",
    //        mpu9250_data.accel_x, mpu9250_data.accel_y, mpu9250_data.accel_z,
    //        mpu9250_data.gyro_x, mpu9250_data.gyro_y, mpu9250_data.gyro_z);
}

// -------------------------------------------------------------------
// --- VERİ HAZIR MI KONTROLÜ ---
// -------------------------------------------------------------------
uint8_t MPU9250_IsDataReady(void)
{
    return mpu9250_data_ready;
}

// -------------------------------------------------------------------
// --- VERİYİ ALMA ---
// -------------------------------------------------------------------
MPU9250_Data_t* MPU9250_GetData(void)
{
    mpu9250_data_ready = 0; // Veri alındı, flag'i temizle
    return &mpu9250_data;
}

// -------------------------------------------------------------------
// --- I2C HATA CALLBACK ---
// -------------------------------------------------------------------
void MPU9250_Error_Callback(void)
{
    printf("MPU9250: I2C/DMA hatasi! Veri okunamadi.\r\n");
    g_system_state.error_flags |= ERR_FLAG_MPU_FAIL;
}
