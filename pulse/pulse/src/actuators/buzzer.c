// pulse/pulse/src/actuators/buzzer.c
#include "buzzer.h" // "actuators/buzzer.h" yerine
#include "shared_data.h"

// DİKKAT: CubeMX'te pini "BUZZER" olarak etiketleyin!
#ifndef BUZZER_Pin
    // Varsayılan (Tanımlı değilse kod patlamasın)
#endif

void BUZZER_Init(void)
{
    // Başlangıçta sustur
    #ifdef BUZZER_Pin
        HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
    #endif
    shared_data.actuators.buzzer_state = 0;
}

// Kısa bip sesi (Blocking - Kod burada durur, sadece açılışta kullan!)
void BUZZER_Beep(uint16_t duration_ms)
{
    #ifdef BUZZER_Pin
        HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
        shared_data.actuators.buzzer_state = 1;
        
        HAL_Delay(duration_ms);
        
        HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
        shared_data.actuators.buzzer_state = 0;
    #endif
}

// Alarmı aç veya kapat (Non-blocking)
void BUZZER_SetState(bool state)
{
    #ifdef BUZZER_Pin
        HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
    #endif
    shared_data.actuators.buzzer_state = state ? 1 : 0;
}
