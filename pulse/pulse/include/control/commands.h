/*#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdint.h>
#include <stdbool.h>

#define CMD_OPEN_LED  0x01
#define CMD_CLOSE_LED 0x02

typedef struct {
    uint8_t command_id;
    uint8_t payload[15];
} command_t;

bool process_command(uint8_t *data);
void execute_open_led(void);
void execute_close_led(void);

#endif*/

// Core/Inc/commands.h
/*
#ifndef INC_COMMANDS_H_
#define INC_COMMANDS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "communication_ids.h"

// Gömülü sisteme gelen komut paketini işleyen ana fonksiyon
void COMMAND_ProcessIncoming(uint8_t id, uint8_t type, uint8_t *payload, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* INC_COMMANDS_H_ */ */


// Core/Inc/commands.h

#ifndef INC_COMMANDS_H_
#define INC_COMMANDS_H_

#include "main.h"
#include "communication_ids.h"

// Komut işleme fonksiyonu
void COMMAND_ProcessIncoming(uint8_t id, uint8_t type, uint8_t *payload, uint16_t len);

// Simulasyon Aktüatör Fonksiyonları
void BRAKE_SetForce(uint8_t force);
void MOTOR_SetTargetSpeed(float speed_mps);
void SYSTEM_EmergencyPowerCut(void);

#endif /* INC_COMMANDS_H_ */
