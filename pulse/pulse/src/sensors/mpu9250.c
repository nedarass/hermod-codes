// Core/Src/mpu9250.c
#include "mpu9250.h"
#include "shared_data.h" 
#include <stdio.h>
#include <string.h>

// MPU9250 veri buffer'ı
static MPU9250_Data_t mpu9250_data;
static uint8_t mpu9250_data_ready = 0;
static I2C_HandleTypeDef* mpu9250_hi2c = NULL; // Sensörün bağlı olduğu I2C birimini (örn: I2C1) tutan değişken.
// Başlangıçta NULL yapıyoruz ki Init çağrılmadan kullanılırsa hata verelim

// DMA için buffer
static uint8_t mpu9250_rx_buffer[14];
// Verileri anlamlı hale getirmek için geçici yapı
static MPU9250_Data_t raw_data;

// -------------------------------------------------------------------
// --- DÜŞÜK SEVİYE I2C FONKSİYONLARI ---
// -------------------------------------------------------------------

// I2C'ye 1 byte yaz
static HAL_StatusTypeDef MPU9250_WriteByte(uint8_t reg, uint8_t data)
{
    return HAL_I2C_Mem_Write(mpu9250_hi2c, MPU9250_I2C_ADDR, reg, 
                           I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
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
    uint8_t whoami;
    
    if (hi2c == NULL) {
        return HAL_ERROR;
    }
    
    mpu9250_hi2c = hi2c;
    
   // 2. Cihaz Kimlik Kontrolü (WHO_AM_I)
    // Sensöre "Sen kimsin?" diye soruyoruz.
    uint8_t whoami;
    if (MPU9250_ReadBytes(MPU9250_WHO_AM_I, &whoami, 1) != HAL_OK) return HAL_ERROR;
    
    // Eğer cevap 0x71 (MPU9250 ID) değilse, yanlış sensör veya bozuk sensör demektir.
    if (whoami != MPU9250_WHO_AM_I_VALUE) return HAL_ERROR;
    
    printf("MPU9250: Cihaz bulundu (WHO_AM_I: 0x%02X)\r\n", whoami);
    
   // 3. Sensörü Resetle (Fabrika Ayarlarına Dön)
    // PWR_MGMT_1 register'ının 7. bitini 1 yaparsak reset atar.
    MPU9250_WriteByte(MPU9250_PWR_MGMT_1, 0x80);
    HAL_Delay(100); // Resetin tamamlanması için bekle
    
   // 4. Saat Kaynağını Ayarla (Clock Source)
    // Daha kararlı çalışması için PLL (Phase Locked Loop) seçiyoruz.
    MPU9250_WriteByte(MPU9250_PWR_MGMT_1, 0x01);
    
    // 5. İvmeölçer Ayarı (ACCEL CONFIG) - KRİTİK AYAR
    // Register: 0x1C. Bit 3 ve 4 hassasiyeti belirler.
    // 0x00 = ±2g  (Hassas ama çabuk doyuma ulaşır)
    // 0x08 = ±4g  (Hyperloop kalkış/fren ivmesi için ideal)
    // 0x10 = ±8g
    // 0x18 = ±16g
    MPU9250_WriteByte(MPU9250_ACCEL_CONFIG, 0x08); 

    // 6. Jiroskop Ayarı (GYRO CONFIG)
    // 0x00 = ±250 derece/saniye (Yeterli hassasiyet)
    MPU9250_WriteByte(MPU9250_GYRO_CONFIG, 0x00);
    
    // 7. Kesme (Interrupt) Ayarları
    // Veri hazır olduğunda INT pini aktif olsun mu? Evet.
    MPU9250_WriteByte(MPU9250_INT_ENABLE, 0x01);

    // Başarılı
    return HAL_OK;
}

// -----------------------------------------------------------------------------
// VERİ OKUMA TETİKLEYİCİSİ (Non-Blocking / DMA)
// -----------------------------------------------------------------------------
/**
 * @brief Bu fonksiyon `main` döngüsü içinde periyodik olarak (örn: 10ms) çağrılmalıdır.
 * İşlemciyi durdurmadan (DMA ile) veri okumayı başlatır.
 */
HAL_StatusTypeDef MPU9250_Trigger_Read(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == NULL) return HAL_ERROR;
    
    // MPU9250_ACCEL_XOUT_H (0x3B) adresinden başlayarak 14 byte oku.
    // Bu işlem arka planda başlar. Bittiğinde Callback fonksiyonu çağrılır.
    return HAL_I2C_Mem_Read_DMA(hi2c, MPU9250_I2C_ADDR, MPU9250_ACCEL_XOUT_H, 
                                I2C_MEMADD_SIZE_8BIT, mpu9250_rx_buffer, 14);
}
// -----------------------------------------------------------------------------
// DMA İŞLEMİ BİTTİĞİNDE ÇALIŞAN FONKSİYON (Callback)
// -----------------------------------------------------------------------------
/**
 * @note  Bu fonksiyonu `main.c` içindeki `HAL_I2C_MemRxCpltCallback` içinden çağırmalısın!
 */
void MPU9250_Read_DMA_Complete_Callback(void)
{
    // --- 1. Adım: Veri Birleştirme (Parsing) ---
    // Sensör verileri 16-bittir ama I2C 8-bit taşır.
    // Yüksek (High) ve Düşük (Low) byte'ları birleştiriyoruz.
    // Örn: High=0x12, Low=0x34 -> Sonuç=0x1234
    
    // İvme (Accelerometer)
    raw_data.accel_x = (int16_t)((mpu9250_rx_buffer[0] << 8) | mpu9250_rx_buffer[1]);
    raw_data.accel_y = (int16_t)((mpu9250_rx_buffer[2] << 8) | mpu9250_rx_buffer[3]);
    raw_data.accel_z = (int16_t)((mpu9250_rx_buffer[4] << 8) | mpu9250_rx_buffer[5]);
    
    // Sıcaklık (Temperature)
    raw_data.temp    = (int16_t)((mpu9250_rx_buffer[6] << 8) | mpu9250_rx_buffer[7]);
    
    // Jiroskop (Gyroscope)
    raw_data.gyro_x  = (int16_t)((mpu9250_rx_buffer[8] << 8) | mpu9250_rx_buffer[9]);
    raw_data.gyro_y  = (int16_t)((mpu9250_rx_buffer[10] << 8) | mpu9250_rx_buffer[11]);
    raw_data.gyro_z  = (int16_t)((mpu9250_rx_buffer[12] << 8) | mpu9250_rx_buffer[13]);


    // --- 2. Adım: Fiziksel Çevrim ve Shared Data Yazma ---
    
    // İvme Hesabı: (Ham Veri / Scale) * 9.81
    // Sonuç m/s^2 cinsinden olur.
    shared_data.sensors.mpu.accel_x_mss = ((float)raw_data.accel_x / MPU9250_ACCEL_SCALE_4G) * GRAVITY_MSS;
    shared_data.sensors.mpu.accel_y_mss = ((float)raw_data.accel_y / MPU9250_ACCEL_SCALE_4G) * GRAVITY_MSS;
    shared_data.sensors.mpu.accel_z_mss = ((float)raw_data.accel_z / MPU9250_ACCEL_SCALE_4G) * GRAVITY_MSS;

    // Gyro Hesabı: Ham Veri / Scale
    // Sonuç derece/saniye cinsinden olur.
    shared_data.sensors.mpu.gyro_x_dps = (float)raw_data.gyro_x / MPU9250_GYRO_SCALE_250;
    shared_data.sensors.mpu.gyro_y_dps = (float)raw_data.gyro_y / MPU9250_GYRO_SCALE_250;
    shared_data.sensors.mpu.gyro_z_dps = (float)raw_data.gyro_z / MPU9250_GYRO_SCALE_250;

    // Sıcaklık Hesabı: Datasheet formülü
    shared_data.sensors.mpu.temp_c = ((float)raw_data.temp - 0) / 333.87f + 21.0f;
}
