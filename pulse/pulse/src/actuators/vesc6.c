
#include "actuators/vesc6.h"
#include "shared_data.h"
#include <string.h>

static UART_HandleTypeDef *vesc_huart = NULL;
static VESC_Status_t vesc_status = {0};
static uint32_t last_vesc_communication = 0;

// --- VESC_CalculateChecksum --- VESC protokolüne özel CRC (Hata doğrulama) hesaplar.
// Verinin yolda bozulup bozulmadığını kontrol eder.
static uint16_t VESC_CalculateChecksum(uint8_t *data, uint16_t len)
{
    uint16_t crc = 0;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x8000) crc = (crc << 1) ^ 0x1021;
            else crc <<= 1;
        }
    }
    return crc;
}

// --- VESC_SendPacket --- Veriyi VESC'in anlayacağı paket formatına sokar:
// [BAŞLANGIÇ] [UZUNLUK] [VERİ] [CRC_H] [CRC_L] [BİTİŞ]
static bool VESC_SendPacket(uint8_t *payload, uint16_t len)
{
    if (vesc_huart == NULL || payload == NULL || len > 250) return false;

    uint8_t buffer[256];
    uint16_t index = 0;

    buffer[index++] = 0x02; // Başlangıç
    buffer[index++] = len;  // Uzunluk

    memcpy(&buffer[index], payload, len); // Veriyi kopyala
    index += len;

    uint16_t crc = VESC_CalculateChecksum(payload, len);
    buffer[index++] = (crc >> 8) & 0xFF; // CRC Yüksek Byte
    buffer[index++] = crc & 0xFF;        // CRC Düşük Byte
    
    buffer[index++] = 0x03; // Bitiş

    // Paketi gönder
    HAL_StatusTypeDef status = HAL_UART_Transmit(vesc_huart, buffer, index, 100);
    
    if (status == HAL_OK) {
        last_vesc_communication = HAL_GetTick();
        return true;
    }
    return false;
}

// --- VESC_Init --- VESC bağlantısını başlatır ve durum değişkenlerini sıfırlar.
void VESC_Init(UART_HandleTypeDef *huart)
{
    vesc_huart = huart;
    memset(&vesc_status, 0, sizeof(VESC_Status_t));
}

// --- VESC_SetRPM --- Hız komutunu gönderir. Sayıyı byte'lara bölerek (Big Endian) paketler.
bool VESC_SetRPM(int32_t rpm)
{
    if (vesc_huart == NULL) return false;
    
    uint8_t payload[5];
    payload[0] = 0x08; // VESC Komutu: RPM Ayarla
    
    // 32-bit sayıyı 4 tane 8-bitlik parçaya böl
    payload[1] = (rpm >> 24) & 0xFF;
    payload[2] = (rpm >> 16) & 0xFF;
    payload[3] = (rpm >> 8) & 0xFF;
    payload[4] = rpm & 0xFF;

    bool success = VESC_SendPacket(payload, 5);
    
    if (success) {
        shared_data.actuators.vesc_status.rpm = rpm; // Son durumu kaydet
    }
    return success;
}

// --- VESC_SetCurrent --- Tork (Akım) komutunu gönderir.
bool VESC_SetCurrent(int32_t current_ma)
{
    if (vesc_huart == NULL) return false;
    
    // Aşırı akım koruması (Yazılımsal Limit)
    if (current_ma < -20000 || current_ma > 20000) return false;
    
    uint8_t payload[5];
    payload[0] = 0x06; // VESC Komutu: Akım Ayarla
    
    payload[1] = (current_ma >> 24) & 0xFF;
    payload[2] = (current_ma >> 16) & 0xFF;
    payload[3] = (current_ma >> 8) & 0xFF;
    payload[4] = current_ma & 0xFF;

    bool success = VESC_SendPacket(payload, 5);
    
    if (success) {
        shared_data.actuators.vesc_status.current = current_ma;
    }
    return success;
}




































/*#include "../../include/actuators/vesc6.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

void vesc6_configure()
{
    // uart baslat
    uart_init(UART_ID, BAUD_RATE);
    
    // pinleri belirle
    gpio_set_function(VESC6_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(VESC6_RX_PIN, GPIO_FUNC_UART);
}

uint8_t calculate_checksum(uint8_t *data, int len) {
    uint8_t checksum = 0;
    for (int i = 1; i < len - 1; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

//test edicez vesc6 protokole internetten baktim
void set_motor_rpm(int rpm)
{
    uint8_t buffer[8];

    buffer[0] = 2;
    buffer[1] = 4; //veri uzunlugu
    buffer[2] = 3; // vesc6 komut kodu (COMM_Set_RPM)
    buffer[3] = (rpm >> 24) & 0xFF;
    buffer[4] = (rpm >> 16) & 0xFF;
    buffer[5] = (rpm >> 8) & 0xFF;
    buffer[6] = rpm & 0xFF;
    buffer[7] = calculate_checksum(buffer, 8);

    uart_write_blocking(UART_ID, buffer, 8);
}
*/
