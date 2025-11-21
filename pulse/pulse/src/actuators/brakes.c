
#include "actuators/brakes.h"
#include "shared_data.h"
#include <string.h>

// Güvenlik için varsayılan olarak frenler SIKILI (ENGAGED) başlar.
static BrakeState_t current_brake_state = BRAKE_ENGAGED; 
static BrakeError_t last_brake_error = BRAKE_ERROR_NONE;
static uint32_t brake_engagement_time = 0;

// --- BRAKES_SetState --- Frenleri sıkmaya (ENGAGED) veya serbest bırakmaya (RELEASED) yarar.
// GÜVENLİK KONTROLÜ: Eğer sistem "Acil Durum"daysa frenlerin açılmasına İZİN VERMEZ.
void BRAKES_SetState(BrakeState_t state)
{
    // Acil durum kontrolü (Safety First)
    if (shared_data.system.emergency_mode && state != BRAKE_ENGAGED) {
        state = BRAKE_ENGAGED; // Zorla frenle
    }
    
    // Donanıma göre mantık: (Genelde LOW = Frenle, HIGH = Bırak)
    GPIO_PinState pin_val = (state == BRAKE_RELEASED) ? GPIO_PIN_SET : GPIO_PIN_RESET;

    // Tanımlı tüm fren pinlerini tetikle
#ifdef BRAKE_1_Pin
    HAL_GPIO_WritePin(BRAKE_1_GPIO_Port, BRAKE_1_Pin, pin_val);
#endif

#ifdef BRAKE_2_Pin  
    HAL_GPIO_WritePin(BRAKE_2_GPIO_Port, BRAKE_2_Pin, pin_val);
#endif

    current_brake_state = state;
    brake_engagement_time = HAL_GetTick();
    
    // Durumu raporla
    shared_data.actuators.brake_state = current_brake_state;
    shared_data.actuators.brake_error = last_brake_error;
    shared_data.actuators.brake_engagement_time = brake_engagement_time;
}

// --- BRAKES_EmergencyEngage ---Yüksek öncelikli ACİL FREN fonksiyonu. Yazılımsal kontrolleri atlayıp donanımı doğrudan kilitler.
// Sistemi "Acil Durum Modu"na sokar.
void BRAKES_EmergencyEngage(void)
{
#ifdef BRAKE_1_Pin
    HAL_GPIO_WritePin(BRAKE_1_GPIO_Port, BRAKE_1_Pin, GPIO_PIN_RESET);
#endif
#ifdef BRAKE_2_Pin
    HAL_GPIO_WritePin(BRAKE_2_GPIO_Port, BRAKE_2_Pin, GPIO_PIN_RESET);
#endif

    current_brake_state = BRAKE_ENGAGED;
    brake_engagement_time = HAL_GetTick();
    
    // Global acil durum bayrağını kaldır!
    shared_data.system.emergency_mode = true;
    shared_data.actuators.brake_state = BRAKE_ENGAGED;
}

// --- BRAKES_IsEngaged --- Frenlerin şu an kilitli olup olmadığını sorgular.
bool BRAKES_IsEngaged(void)
{
    return (current_brake_state == BRAKE_ENGAGED);
}

// --- BRAKES_GetLastError --- Fren sistemindeki son hatayı döndürür.
BrakeError_t BRAKES_GetLastError(void)
{
    return last_brake_error;
}




















/*#include "pico/stdlib.h"
#include <stdio.h>
#include "../../include/default_pins.h"
#include "../../include/actuators/brakes.h"

void brakes_configure_pins()
{
    gpio_init(BRAKE_0_DIGITAL);
    gpio_set_dir(BRAKE_0_DIGITAL, GPIO_OUT);

    gpio_init(BRAKE_1_DIGITAL);
    gpio_set_dir(BRAKE_1_DIGITAL, GPIO_OUT);

    gpio_init(BRAKE_2_DIGITAL);
    gpio_set_dir(BRAKE_2_DIGITAL, GPIO_OUT);

    gpio_init(BRAKE_3_DIGITAL);
    gpio_set_dir(BRAKE_3_DIGITAL, GPIO_OUT);
}

void set_brakes(uint brake_pin, enum gpio_dir pin_conf)
{
    gpio_put(brake_pin, pin_conf);
}
*/
