
#include "actuators/inverter_rs485.h"
#include "shared_data.h"
#include <string.h>

static UART_HandleTypeDef *rs485_huart = NULL;
static InverterState_t inverter_state = INVERTER_STATE_DISABLED;
static uint32_t last_communication_time = 0;

// --- MODBUS_CRC16 --- Gönderilen verinin bozulup bozulmadığını anlamak için matematiksel imza (Checksum) hesaplar.
// Modbus protokolü standardıdır.
static uint16_t MODBUS_CRC16(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFF;
    while (length--) {
        crc ^= *data++;
        for (int i = 0; i < 8; i++) {
            if (crc & 0x0001) crc = (crc >> 1) ^ 0xA001;
            else crc = crc >> 1;
        }
    }
    return crc;
}

// --- INVERTER_Init --- UART bağlantısını alır ve RS485 çipini "Dinleme" (Receive) moduna alır.
void INVERTER_Init(UART_HandleTypeDef *huart)
{
    if (huart == NULL) return;
    rs485_huart = huart;
    HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET); // Dinleme Modu
    inverter_state = INVERTER_STATE_READY;
    shared_data.actuators.inverter_state = inverter_state;
}

// --- INVERTER_Write --- RS485 üzerinden veri gönderir.
// ÖNEMLİ: Göndermeden önce DE pinini açar (Transmitter On), bitince kapatır (Receiver On).
bool INVERTER_Write(uint8_t *data, uint16_t len)
{
    if (rs485_huart == NULL || data == NULL) return false;

    // 1. Konuşma Moduna Geç (TX Enable)
    HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET);
    
    // Kısa bekleme (Çipin uyanması için)
    for (volatile int i = 0; i < 100; i++); 
    
    // 2. Veriyi Gönder
    HAL_StatusTypeDef status = HAL_UART_Transmit(rs485_huart, data, len, 100);

    // 3. Veri bitene kadar bekle ve Dinleme Moduna Geç
    for (volatile int i = 0; i < 100; i++); 
    HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET);

    if (status == HAL_OK) {
        last_communication_time = HAL_GetTick();
        return true;
    }
    
    inverter_state = INVERTER_STATE_ERROR;
    shared_data.actuators.inverter_state = inverter_state;
    return false;
}

// --- INVERTER_SetRPM --- İstenilen hızı Modbus paketine çevirir, CRC ekler ve gönderir.
bool INVERTER_SetRPM(int16_t rpm)
{
    if (rs485_huart == NULL) return false;
    
    // Mantıksız hızları engelle (Güvenlik)
    if (rpm < -10000 || rpm > 10000) return false; 
    
    // Modbus Paketi Oluşturma (Örnek yapı)
    uint8_t frame[8];
    frame[0] = 0x01; // Sürücü Adresi
    frame[1] = 0x06; // Yazma Komutu
    frame[2] = 0x20; // Register Adresi Yüksek
    frame[3] = 0x00; // Register Adresi Düşük
    frame[4] = (rpm >> 8) & 0xFF; // Veri Yüksek
    frame[5] = rpm & 0xFF;        // Veri Düşük
    
    // İmzayı (CRC) hesapla ve ekle
    uint16_t crc = MODBUS_CRC16(frame, 6);
    frame[6] = crc & 0xFF;
    frame[7] = (crc >> 8) & 0xFF;

    bool success = INVERTER_Write(frame, 8);
    
    if (success) {
        inverter_state = INVERTER_STATE_RUNNING;
        shared_data.actuators.target_rpm = rpm;
        shared_data.actuators.inverter_state = inverter_state;
    }
    return success;
}

// --- INVERTER_GetState --- Sürücü durumunu döndürür. Eğer uzun süre iletişim yoksa HATA verir.
InverterState_t INVERTER_GetState(void)
{
    if ((HAL_GetTick() - last_communication_time) > 1000) { // 1 saniye sessizlik = Hata
        inverter_state = INVERTER_STATE_ERROR;
    }
    return inverter_state;
}






























/*#include "../../include/actuators/inverter_rs485.h"

void rs485_modbus_configure()
{
    // uart baslat
    uart_init(UART_ID, BAUD_RATE);
    
    // pinleri belirle
    gpio_set_function(RS485_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(RS485_RX_PIN, GPIO_FUNC_UART);
    
    // rs485 yönlendirme pini çıkış olarak ayarlanıyor
    gpio_init(RS485_DE_PIN);
    gpio_set_dir(RS485_DE_PIN, GPIO_OUT);

    gpio_put(RS485_DE_PIN, 1);  // de yuksek = tx aktif
    sleep_us(RS485_DELAY_US);   // mod gecisi icin transceiver bekliyoruz


    // modbus icin ayarmis
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);

    // fifo
    //uart_set_fifo_enabled(UART_ID, true);
}

void rs485_write(uint8_t *message, uint8_t len)
{
    // veriyi yaz
    uart_puts(UART_ID, message);
    
    // veri gonderimini bekle
    while (!uart_is_writable(UART_ID)) {
        tight_loop_contents();
    }
    sleep_ms(RS485_DELAY_US);
}
*/
