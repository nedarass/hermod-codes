// pulse/pulse/src/actuators/cooling.c
#include "cooling.h" // "actuators/cooling.h" yerine
#include "shared_data.h"

// DİKKAT: CubeMX'te pini "FAN_RELAY" olarak etiketleyin!
#ifndef FAN_RELAY_Pin
    // Varsayılan boş tanım
#endif

void COOLING_Init(void)
{
    // Başlangıçta fanlar kapalı
    COOLING_SetFanState(false);
}

void COOLING_SetFanState(bool state)
{
    #ifdef FAN_RELAY_Pin
        HAL_GPIO_WritePin(FAN_RELAY_GPIO_Port, FAN_RELAY_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
    #endif
    
    shared_data.actuators.cooling_state = state ? 1 : 0;
}

// PWM şimdilik iptal edildi (Basitlik için), sadece Aç/Kapa yeterli.
void COOLING_SetFanSpeed(uint8_t speed_percent)
{
    // Eğer fan hızı %50'den büyükse Aç, yoksa Kapat
    COOLING_SetFanState(speed_percent > 50);
}
