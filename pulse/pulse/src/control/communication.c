#include "communication.h"
#include "commands.h"
#include "shared_data.h"
#include <string.h>
#include <stdio.h>



// Harici değişkenler (main.c'de tanimlanan)
extern UART_HandleTypeDef huart1;
extern uint8_t rx_data;

// timeout için 
static uint32_t last_rx_time = 0;
#define COMM_TIMEOUT_MS 1000
#define MAX_FRAME 256


// -------------------------------------------------------------------
// --- ALICI PAKET ÇÖZÜMLEME (PARSER) KODLARI (State Machine) ---
// -------------------------------------------------------------------

// --- STATE MACHINE ---
typedef enum {
    RX_STATE_WAIT_START, RX_STATE_ID, RX_STATE_TYPE,
    RX_STATE_LEN_L, RX_STATE_LEN_H, RX_STATE_PAYLOAD, RX_STATE_END
} rx_state_t;

static rx_state_t rx_state = RX_STATE_WAIT_START;
static uint8_t  rx_id;
static uint8_t  rx_type;
static uint16_t rx_len;
static uint16_t rx_i;
static uint8_t rx_buffer[MAX_FRAME];

// --- INIT ---
void COMM_Init(void)
{
    if (HAL_UART_Receive_IT(&huart1, &rx_data, 1) != HAL_OK)
    {
       shared_data.system.error_flags |= ERR_COMM_TIMEOUT;
    }
    printf("UART Haberlesme Baslatildi.\r\n");
}
/**
 * @brief Paket başarıyla alındığında çağrılır. Komutları işler.
 */
// --- PACKET RECEIVED ---
static void on_packet_received(uint8_t id, uint8_t type, uint8_t *payload, uint16_t len)
{
    // Komut ID'si ise Commands modülüne ilet
    if (id >= CMD_ID_START)
    {
        COMMAND_ProcessIncoming(id, type, payload, len);

        // Ping ise hemen cevap ver
        if (id == CMD_PING_REQUEST) {
            uint16_t ping_resp = 10; // Örnek değer
            COMM_TransmitTelemetry(ID_PING_RESPONSE, TYPE_I16, &ping_resp, sizeof(uint16_t));
        }
    }
}

// ÇÖZÜM: CRC checksum ekle
static uint16_t COMM_CalculateCRC(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x8000) 
                crc = (crc << 1) ^ 0x1021;
            else 
                crc <<= 1;
        }
    }
    return crc;
}


// --- PROCESS BYTE (ISR'dan çağrılır) ---
void COMM_ProcessByte(uint8_t ch)
{
    last_rx_time = HAL_GetTick();
    
    switch (rx_state)
    {
        case RX_STATE_WAIT_START:
            if (ch == PKT_START) rx_state = RX_STATE_ID;
            break;
        case RX_STATE_ID:
            rx_id = ch; rx_state = RX_STATE_TYPE;
            break;
        case RX_STATE_TYPE:
            rx_type = ch; rx_state = RX_STATE_LEN_L;
            break;
        case RX_STATE_LEN_L:
            rx_len = ch; rx_state = RX_STATE_LEN_H;
            break;
        case RX_STATE_LEN_H:
            rx_len |= ((uint16_t)ch << 8);
            if (rx_len > MAX_FRAME) rx_state = RX_STATE_WAIT_START;
            else if (rx_len == 0) rx_state = RX_STATE_END;
            else { rx_i = 0; rx_state = RX_STATE_PAYLOAD; }
            break;
        case RX_STATE_PAYLOAD:
            rx_buffer[rx_i++] = ch;
            if (rx_i >= rx_len) rx_state = RX_STATE_END;
            break;
        case RX_STATE_END:
            if (ch == PKT_END) {
                on_packet_received(rx_id, rx_type, rx_buffer, rx_len);
            }
            rx_state = RX_STATE_WAIT_START;
            break;
    }
}

// timeout için
// Control loop'ta timeout kontrolü
void COMM_CheckTimeout(void)
{
    if (rx_state != RX_STATE_WAIT_START && 
        (HAL_GetTick() - last_rx_time) > COMM_TIMEOUT_MS) {
        printf("UYARI: Haberlesme timeout, state machine resetlendi\r\n");
        rx_state = RX_STATE_WAIT_START;
    }
}

// -------------------------------------------------------------------
// --- GÖNDERİCİ FONKSİYONU ---
// -------------------------------------------------------------------

void COMM_TransmitTelemetry(uint8_t id, uint8_t type, const void *payload, uint16_t payload_len)
{
    uint8_t frame[MAX_FRAME];
    uint16_t idx = 0;

    // Başlık: [START][ID][TYPE][LEN_L][LEN_H]
    frame[idx++] = PKT_START;
    frame[idx++] = id;
    frame[idx++] = type;
    frame[idx++] = (uint8_t)(payload_len & 0xFF);
    frame[idx++] = (uint8_t)((payload_len >> 8) & 0xFF);

    // 2. Veriyi Kopyala
    if (payload_len > 0 && payload != NULL) {
        memcpy(&frame[idx], payload, payload_len);
        
        // CRC Hesabı (Sadece Payload için - yukarıdaki mantıkla uyumlu olsun diye)
        uint16_t crc = COMM_CalculateCRC((uint8_t*)payload, payload_len);
        
        idx += payload_len;
        
        // 3. CRC Ekle
        frame[idx++] = (uint8_t)(crc & 0xFF);
        frame[idx++] = (uint8_t)((crc >> 8) & 0xFF);
    } else {
        // Payload yoksa CRC 0 veya Header CRC'si olabilir
        frame[idx++] = 0; frame[idx++] = 0; 
    }

    // 4. Bitiş
    frame[idx++] = PKT_END;

    // 5. Gönder (Blocking Mode - 50ms timeout)
    HAL_UART_Transmit(&huart1, frame, idx, 50);

}


// -------------------------------------------------------------------
// --- TELEMERTİ & HEALTH CHECK PAKETLERİ OLUŞTURMA ---
// ---------------------------------------------------

// --- TELEMETRY SENDER ---
void COMM_SendTelemetryData(void)
{
    // Doğru Veri Kaynağı: Shared Data
    COMM_TransmitTelemetry(ID_VELOCITY, TYPE_F32, &shared_data.sensors.nav.velocity_mps, sizeof(float));
    COMM_TransmitTelemetry(ID_POSITION, TYPE_F32, &shared_data.sensors.nav.position_m, sizeof(float));
    COMM_TransmitTelemetry(ID_TEMPERATURE_NTC1, TYPE_F32, &shared_data.sensors.battery.ntc_temp_c, sizeof(float));
    COMM_TransmitTelemetry(ID_CURRENT, TYPE_I16, &shared_data.sensors.battery.current_ma, sizeof(int16_t));
    COMM_TransmitTelemetry(ID_VOLTAGE, TYPE_U16, &shared_data.sensors.battery.voltage_mv, sizeof(uint16_t));
    COMM_TransmitTelemetry(ID_BRAKE_STATUS, TYPE_U8, &shared_data.actuators.brake_state, sizeof(uint8_t));
}

void COMM_SendHealthCheck(void)
{
   // 1. CPU SICAKLIĞI
    // Not: İşlemci içi sıcaklık sensörü henüz aktif değil.
    // İlerde: HAL_ADC_GetValue(&hadc1) ile okunacak. Şimdilik shared_data'dan al.
    COMM_TransmitTelemetry(ID_CPU_TEMP, TYPE_F32, 
                           &shared_data.system.cpu_temp_c, sizeof(float));

    // 2. HATA BAYRAKLARI (En Kritik Veri)
    // 32-bitlik hata maskesini (0x00000001 gibi) gönderiyoruz.
    COMM_TransmitTelemetry(ID_ERROR_FLAG, TYPE_BIN, 
                           &shared_data.system.error_flags, sizeof(uint32_t));

    // 3. GÜÇ HATTI DURUMU (Röle çekili mi?)
    // actuators.power_state verisini (0:OFF, 1:ON) kullanıyoruz.
    COMM_TransmitTelemetry(ID_POWER_LINE_STATUS, TYPE_U8, 
                           &shared_data.actuators.power_state, sizeof(uint8_t));

    // 4. PING / İLETİŞİM GECİKMESİ
    // Son ölçülen ping süresini gönder.
    COMM_TransmitTelemetry(ID_PING_RESPONSE, TYPE_I16, 
                           &shared_data.system.last_ping_ms, sizeof(uint16_t));

    // 5. SİSTEM DURUMU (RTOS Task Yerine)
    // RTOS olmadığı için "Main Loop Çalışıyor" anlamında 1 gönderiyoruz.
    // Eğer error_flags 0 değilse durum 2 (Hata) olarak da gönderilebilir.
    uint8_t status = (shared_data.system.error_flags == 0) ? 1 : 2; 
    COMM_TransmitTelemetry(ID_RTOS_STATUS, TYPE_U8, &status, sizeof(uint8_t));
}
