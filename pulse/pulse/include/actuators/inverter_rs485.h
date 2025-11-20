// Podun ana motor sürücüsü (inverter) ile RS485 protokolü üzerinden iletişim kurarak motorun hızını veya torkunu kontrol eder
// inverter_rs485.h - PROTOCOL ENTEGRASYONLU
#ifndef INVERTER_RS485_H
#define INVERTER_RS485_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "shared_data.h"

// Modbus Timeout
#define MODBUS_TIMEOUT_MS   100

typedef enum {
    INVERTER_STATE_DISABLED = 0,
    INVERTER_STATE_READY = 1,
    INVERTER_STATE_RUNNING = 2,
    INVERTER_STATE_ERROR = 3
} InverterState_t;

void INVERTER_Init(UART_HandleTypeDef *huart);
// Inverter iletişimini başlatır. huart: Kullanılacak UART birimi (örn: &huart2) 
bool INVERTER_Write(uint8_t *data, uint16_t len);
/* Modbus/RS485 üzerinden ham veri paketi gönderir.
 Bu fonksiyon arka planda DE (Driver Enable) pinini yönetmelidir.
 data: Gönderilecek veri dizisi pointer'ı , len: Veri uzunluğu */
bool INVERTER_SetRPM(int16_t rpm);
/* Motor hızını ayarlar (Yardımcı fonksiyon).
  Arka planda doğru Modbus paketini hazırlar ve INVERTER_Write'ı çağırır. rpm: Hedef RPM */
bool INVERTER_ReadStatus(void);
InverterState_t INVERTER_GetState(void);

#ifdef __cplusplus
}
#endif

#endif







/*#ifndef INVERTER_RS485_H
#define INVERTER_RS485_H

#include "pico/stdlib.h"
#include "stdio.h"
#include "../default_pins.h"

void rs485_modbus_configure(); //modbus iletisimi icin konfigurasyon

// rs485 modbus üzerinden veri gönderme fonksiyonu
void rs485_write(uint8_t *message, uint8_t len);

#endif
*/
