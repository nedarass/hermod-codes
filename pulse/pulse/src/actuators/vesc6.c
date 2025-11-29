
// pulse/pulse/src/actuators/vesc6.c
#include "vesc6.h" // Include yolunu düzelttim
#include "shared_data.h"
#include <string.h>

static UART_HandleTypeDef *vesc_huart = NULL;
// static VESC_Status_t vesc_status = {0}; // GEREKSİZ, Shared Data kullanacağız
static uint32_t last_vesc_communication = 0;

// --- VESC_CalculateChecksum (Static - Sadece bu dosyada kullanılır) ---
// CRC-16-CCITT (Poly: 0x1021)
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

// --- VESC_SendPacket ---
// [BAŞLANGIÇ(0x02)] [UZUNLUK] [VERİ] [CRC_H] [CRC_L] [BİTİŞ(0x03)]
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
    // Shared Data içindeki VESC yapısını sıfırla
    memset(&shared_data.actuators.vesc_status, 0, sizeof(VESC_Data_t));
}

// --- VESC_SetRPM --- Hız komutunu gönderir. Sayıyı byte'lara bölerek (Big Endian) paketler.
bool VESC_SetRPM(int32_t rpm)
{
    if (vesc_huart == NULL) return false;
    
    uint8_t payload[5];
    payload[0] = 0x08; // COMM_SET_RPM
    
    // Big Endian (MSB First)
    payload[1] = (rpm >> 24) & 0xFF;
    payload[2] = (rpm >> 16) & 0xFF;
    payload[3] = (rpm >> 8) & 0xFF;
    payload[4] = rpm & 0xFF;

    bool success = VESC_SendPacket(payload, 5);
    
    if (success) {
        // Hedefi kaydet (Telemetri için)
        // DİKKAT: Buraya vesc_status.rpm YAZMIYORUZ. O gerçek hızdır.
        // Buraya sadece hedefimizi (target) yazıyoruz.
        shared_data.actuators.target_rpm = rpm; 
    }
    return success;
}

// --- VESC_SetCurrent --- Tork (Akım) komutunu gönderir.
bool VESC_SetCurrent(int32_t current_ma)
{
    if (vesc_huart == NULL) return false;
    
    // Aşırı akım koruması (Limit Artırıldı: 40A)
    // Hyperloop kalkışında yüksek akım gerekebilir.
    if (current_ma < -40000 || current_ma > 40000) return false; 
    
    uint8_t payload[5];
    payload[0] = 0x06; // COMM_SET_CURRENT
    
    payload[1] = (current_ma >> 24) & 0xFF;
    payload[2] = (current_ma >> 16) & 0xFF;
    payload[3] = (current_ma >> 8) & 0xFF;
    payload[4] = current_ma & 0xFF;

    return VESC_SendPacket(payload, 5);
}

// --- VESC_RequestStatus ---
bool VESC_RequestStatus(void)
{
    if (vesc_huart == NULL) return false;
    uint8_t payload[1];
    payload[0] = 0x04; // COMM_GET_VALUES
    return VESC_SendPacket(payload, 1);
}

// --- VESC_ParseStatus ---
// Bu fonksiyon main.c içinde UART RX Interrupt'ından veya Buffer dolunca çağrılır.
bool VESC_ParseStatus(uint8_t *buffer, uint16_t len)
{
    // Minimum uzunluk kontrolü (Örn: ID + Payload + CRC + End)
    if (len < 10) return false;

    // VESC Paket Başlangıcı Kontrolü
    if (buffer[0] != 0x04) return false; // Paket ID: COMM_GET_VALUES değilse çık

    int32_t ind = 1;

    // --- PROTOKOL AYRIŞTIRMA (PARSING) ---
    // Not: Bu sıralama Standart VESC Firmware 5.x/6.x yapısına göredir.
    
    // 1. Temp MOSFET (2 byte, scale 10)
    // int16_t temp_mos = (int16_t)((buffer[ind] << 8) | buffer[ind+1]); 
    ind += 2;

    // 2. Temp Motor (2 byte, scale 10)
    int16_t temp_motor = (int16_t)((buffer[ind] << 8) | buffer[ind+1]); ind += 2;
    shared_data.actuators.vesc_status.temperature = (float)temp_motor / 10.0f;

    // 3. Motor Current (4 byte, scale 100)
    // DÜZELTME: VESC genelde Centiamps (x100) gönderir. 
    // Bizim shared_data mA tutuyor. 5A -> VESC: 500 -> Biz: 5000mA olmalı.
    // Bu yüzden 10 ile çarpıyoruz. BUNU TEST EDEREK KONTROL ETMEN GEREKİYOR (STM32-VESC kaç katını alıyor) 
    int32_t current = (int32_t)((buffer[ind] << 24) | (buffer[ind+1] << 16) | (buffer[ind+2] << 8) | buffer[ind+3]);
    ind += 4;
    shared_data.actuators.vesc_status.current = current * 10; 

    // 4. Input Current (4 byte) - Atlıyoruz
    ind += 4; 

    // 5. ID (4 byte) - Atlıyoruz
    ind += 4; // Atla

    // 6. Z (4 byte) - Atlıyoruz
    ind += 4; // Atla

    // 7. Input Voltage (2 byte, scale 10)
    int16_t voltage = (int16_t)((buffer[ind] << 8) | buffer[ind+1]); ind += 2;
    shared_data.actuators.vesc_status.voltage = (float)voltage / 10.0f;

    // 8. RPM (4 byte, scale 1)
    int32_t rpm = (int32_t)((buffer[ind] << 24) | (buffer[ind+1] << 16) | (buffer[ind+2] << 8) | buffer[ind+3]);
    ind += 4;
    shared_data.actuators.vesc_status.rpm = rpm;
    
    // Sürücü durumunu güncelle
    shared_data.actuators.inverter_state = 2; // INVERTER_STATE_RUNNING

    return true;
}
