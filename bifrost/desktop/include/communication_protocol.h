// bifrost/desktop/include/communication_protocol.h
// serial ve tcp aynı ıd kodu üzerinden kontrol edilmesi için oluşturuldu

#ifndef COMMUNICATION_PROTOCOL_H
#define COMMUNICATION_PROTOCOL_H

#include <QtGlobal>

// --- FRAME YAPISI ---
// [AA] [55] [ID] [LEN] [DATA...] [CRC]
constexpr quint8 PKT_HEADER_1 = 0xAA;
constexpr quint8 PKT_HEADER_2 = 0x55;

// --- VERİ TİPLERİ ---
constexpr quint8 TYPE_U8  = 0x01;
constexpr quint8 TYPE_F32 = 0x07;

// --- TELEMETRİ ID'LERİ (STM32'den Gelenler) ---
constexpr quint8 ID_VELOCITY       = 0x01;
constexpr quint8 ID_ACCELERATION   = 0x02;
constexpr quint8 ID_POSITION       = 0x03;
constexpr quint8 ID_VOLTAGE        = 0x04;
constexpr quint8 ID_CURRENT        = 0x05;
constexpr quint8 ID_POWER          = 0x06;
constexpr quint8 ID_TEMPERATURE    = 0x07;
constexpr quint8 ID_BRAKE_STATUS   = 0x08;
constexpr quint8 ID_CONN_STATUS    = 0x09;

// Sağlık / Hata ID'leri
constexpr quint8 ID_PING_RESPONSE  = 0xF1;
constexpr quint8 ID_CPU_TEMP       = 0xF2;
constexpr quint8 ID_ERROR_FLAGS    = 0xF3;

// --- KOMUT ID'LERİ (STM32'ye Gidenler) ---
constexpr quint8 CMD_BRAKE_ACTUATE    = 0xA1;
constexpr quint8 CMD_SET_TARGET_SPEED = 0xA2;
constexpr quint8 CMD_POWER_CUT_OFF    = 0xA3;
constexpr quint8 CMD_PING_REQUEST     = 0xA4;

#endif // COMMUNICATION_PROTOCOL_H
