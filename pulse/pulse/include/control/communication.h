
#ifndef INC_COMMUNICATION_H_
#define INC_COMMUNICATION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "communication_ids.h"

// Harici UART Handle'ı (main.c'de tanımlanır)
extern UART_HandleTypeDef huart1;
extern uint8_t rx_data;


// Fonksiyon prototipleri
void COMM_Init(void); // UART kesmeli alımı başlatır
void COMM_ProcessByte(uint8_t ch); // UART RxCpltCallback'dan çağrılır

// Telemetri Gönderimi
void COMM_TransmitTelemetry(uint8_t id, uint8_t type, const void *payload, uint16_t payload_len);
void COMM_SendHealthCheck(void);
void COMM_SendTelemetryData(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_COMMUNICATION_H_ */
