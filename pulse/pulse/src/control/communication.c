/*#include "../../include/control/communication.h"
#include "../../include/control/control.h"
#include "tusb.h"
#include "tusb_config.h"
#include <string.h>

#define BUFFER_SIZE 128

volatile uint8_t usb_buffer[BUFFER_SIZE];  // gelen veriler burada.

void buffer_clear()
{
    memset((void*)usb_buffer, 0, BUFFER_SIZE);
}

int read_protocol_data()
{
    if(!tud_cdc_available())
    {
        return 0;
    }

    int bytes_read = (int)tud_cdc_read((void *)usb_buffer, BUFFER_SIZE - 1);
    if (bytes_read > 0) {
        usb_buffer[bytes_read] = '\0';  // null terminasyonu yap cstr
    }

    return bytes_read;  // okunan byte sayisi
}

void write_protocol_data(uint8_t* data, int size)
{
    if (size > 0) {
        tud_cdc_write(data, size);
        tud_cdc_write_flush();  // tusb ic bufferda tamamlanmayan kismi aninda gonderir
    }
}
*/
// Core/Src/communication.c

#include "communication.h"
#include "commands.h"
#include "shared_data.h"
#include <string.h>
#include <stdio.h>

// timeout için 
static uint32_t last_rx_time = 0;
#define COMM_TIMEOUT_MS 1000
// Harici değişkenler (main.c'de tanimlanan)
extern UART_HandleTypeDef huart1;
extern uint8_t rx_data;
// Global sistem durum yapısı
extern System_State_t g_system_state;

// -------------------------------------------------------------------
// --- ALICI PAKET ÇÖZÜMLEME (PARSER) KODLARI (State Machine) ---
// -------------------------------------------------------------------

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


/**
 * @brief Paket başarıyla alındığında çağrılır. Komutları işler.
 */
static void on_packet_received(uint8_t id, uint8_t type, uint8_t *payload, uint16_t len)
{
    // CMD_ID_START'tan büyük veya eşit ID'ler komut kabul edilir.
    if (id >= CMD_ID_START)
    {
        COMMAND_ProcessIncoming(id, type, payload, len);

        // Ping cevabını hemen gönder
        if (id == CMD_PING_REQUEST) {
            uint16_t ping_time_ms = 10; // Simülasyon ping süresi
            g_system_state.ping_ms = ping_time_ms;
            COMM_TransmitTelemetry(ID_PING_RESPONSE, TYPE_I16, &ping_time_ms, sizeof(uint16_t));
        }
    }
    else
    {
        // Gömülü sistemin başka bir gömülü sistemden telemetri alması durumu
        // Şu anki senaryoda bu beklenmez.
        printf("UYARI: Tanimsiz Gelen Paket (ID:0x%02X)\r\n", id);
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


// Paket formatını güncelle: [START][ID][TYPE][LEN_L][LEN_H][PAYLOAD][CRC_L][CRC_H][END]
/**
 * @brief UART'tan gelen her bir baytı işleyen durum makinesi.
 */
void COMM_ProcessByte(uint8_t ch)
{
    // timeoout için 
     last_rx_time = HAL_GetTick();
    switch (rx_state)
    {
        case RX_STATE_WAIT_START:
            if (ch == PKT_START) { rx_state = RX_STATE_ID; }
            break;
        case RX_STATE_ID:
            rx_id = ch; rx_state = RX_STATE_TYPE; break;
        case RX_STATE_TYPE:
            rx_type = ch; rx_state = RX_STATE_LEN_L; break;
        case RX_STATE_LEN_L:
            rx_len = ch; rx_state = RX_STATE_LEN_H; break;
        case RX_STATE_LEN_H:
            rx_len |= ((uint16_t)ch << 8);

            if (rx_len > MAX_FRAME || rx_len == 0xFFFF) {
                rx_state = RX_STATE_WAIT_START;
            }
            else if (rx_len == 0) {
                rx_state = RX_STATE_END;
            }
            else {
                rx_i = 0; rx_state = RX_STATE_PAYLOAD;
            }
            break;
        case RX_STATE_PAYLOAD:
            rx_buffer[rx_i++] = ch;
            if (rx_i >= rx_len) { rx_state = RX_STATE_END; }
            break;
        case RX_STATE_END:
            if (ch == PKT_END) {
                on_packet_received(rx_id, rx_type, rx_buffer, rx_len);
            }
            rx_state = RX_STATE_WAIT_START; break;
        default:
            rx_state = RX_STATE_WAIT_START; break;
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
/**
 * @brief UART Alım kesmesini başlatır. main.c'den çağrılır.
 */
void COMM_Init(void)
{
    if (HAL_UART_Receive_IT(&huart1, &rx_data, 1) != HAL_OK)
    {
      g_system_state.error_flags |= ERR_FLAG_COMM_TIMEOUT;
      Error_Handler();
    }
    printf("UART Haberlesme Baslatildi. Komut Bekleniyor...\r\n");
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

    // Payload
    if (payload_len > 0 && payload != NULL) {
        memcpy(&frame[idx], payload, payload_len);
        idx += payload_len;
    }

    // Bitiş
    frame[idx++] = PKT_END;

    HAL_UART_Transmit(&huart1, frame, idx, 50);
}


// -------------------------------------------------------------------
// --- TELEMERTİ & HEALTH CHECK PAKETLERİ OLUŞTURMA ---
// ---------------------------------------------------

void COMM_SendTelemetryData(void)
{
    // Monitoring Page Verileri (Hızlı döngüde gönderilir)

    COMM_TransmitTelemetry(ID_VELOCITY, TYPE_F32, &g_system_state.velocity_mps, sizeof(float));
    COMM_TransmitTelemetry(ID_POSITION, TYPE_F32, &g_system_state.position_m, sizeof(float));
    COMM_TransmitTelemetry(ID_TEMPERATURE_NTC1, TYPE_I16, &g_system_state.raw_temp_ntc1, sizeof(int16_t));
    COMM_TransmitTelemetry(ID_CURRENT, TYPE_I16, &g_system_state.raw_current_ma, sizeof(int16_t));
    COMM_TransmitTelemetry(ID_POWER, TYPE_F32, &g_system_state.power_w, sizeof(float));
    COMM_TransmitTelemetry(ID_BRAKE_STATUS, TYPE_U8, &g_system_state.brake_status, sizeof(uint8_t));

    // Not: ID_VOLTAGE burada uint16'dan Type_U8'e çevrilerek gönderildi,
    // ancak C++ tarafında uint16 bekleniyor. En doğru format:
    COMM_TransmitTelemetry(ID_VOLTAGE, TYPE_I16, &g_system_state.raw_voltage_mv, sizeof(uint16_t));

}


void COMM_SendHealthCheck(void)
{
    // Hermod Pulse Page Verileri (Yavaş döngüde gönderilir)

    COMM_TransmitTelemetry(ID_CPU_TEMP, TYPE_F32, &g_system_state.cpu_temp_c, sizeof(float));
    COMM_TransmitTelemetry(ID_ERROR_FLAG, TYPE_BIN, &g_system_state.error_flags, sizeof(uint32_t));
    COMM_TransmitTelemetry(ID_POWER_LINE_STATUS, TYPE_U8, &g_system_state.power_line_status, sizeof(uint8_t));
    COMM_TransmitTelemetry(ID_PING_RESPONSE, TYPE_I16, &g_system_state.ping_ms, sizeof(uint16_t));
    COMM_TransmitTelemetry(ID_RTOS_STATUS, TYPE_U8, &g_system_state.rtos_task_status, sizeof(uint8_t));
}
