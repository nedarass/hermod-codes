
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
