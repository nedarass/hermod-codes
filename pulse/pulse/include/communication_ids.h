// Core/Inc/communication_ids.h

#ifndef INC_COMMUNICATION_IDS_H_
#define INC_COMMUNICATION_IDS_H_

#include <stdint.h>

// -------------------------------------------------------------------
// --- PAKET SINIRLARI & TİPLERİ ---
// -------------------------------------------------------------------
#define PKT_START  0xAA
#define PKT_END    0x55
#define MAX_FRAME  600

typedef enum {
    TYPE_U8   = 0x01,
    TYPE_I16  = 0x04,
    TYPE_F32  = 0x07,
    TYPE_STR  = 0x10,
    TYPE_BIN  = 0xFF
} payload_type_t;


// -------------------------------------------------------------------
// --- TELEMETRİ ID'LERİ (Pulse -> Bifrost) ---
// Not: 0x01-0x08 mevcut serial_communicator.cpp ile uyumludur.
// -------------------------------------------------------------------
#define ID_VELOCITY             0x01 // float
#define ID_ACCELERATION         0x02 // float
#define ID_POSITION             0x03 // float
#define ID_VOLTAGE              0x04 // uint16 (mV)
#define ID_CURRENT              0x05 // int16 (mA)
#define ID_POWER                0x06 // float (W)
#define ID_TEMPERATURE_NTC1     0x07 // int16 (C * 100)
#define ID_BRAKE_STATUS         0x08 // uint8 (0/1)

// HEALTH CHECK ID'LERİ (0xF0'dan başlar)
#define ID_PING_RESPONSE        0xF1 // uint16 (ms)
#define ID_CPU_TEMP             0xF2 // float (C)
#define ID_ERROR_FLAG           0xF3 // uint32 (Bitmask)
#define ID_POWER_LINE_STATUS    0xF4 // uint8 (0: OFF, 1: ON)
#define ID_RTOS_STATUS          0xF5 // uint8 (Task Durumu)


// -------------------------------------------------------------------
// --- KOMUT ID'LERİ (Bifrost -> Pulse) ---
// -------------------------------------------------------------------
#define CMD_ID_START            0xA0
#define CMD_BRAKE_ACTUATE       0xA1 // Payload: uint8_t force (0-100)
#define CMD_SET_TARGET_SPEED    0xA2 // Payload: float speed (m/s)
#define CMD_POWER_CUT_OFF       0xA3 // Payload: yok
#define CMD_PING_REQUEST        0xA4 // Payload: yok

#endif /* INC_COMMUNICATION_IDS_H_ */
