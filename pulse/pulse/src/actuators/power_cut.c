#include "actuators/brakes.h" // yeni ekledim brakes_emergencyengage kullanabilmek için 
#include "actuators/power_cut.h"
#include "shared_data.h"

static PowerState_t power_state = POWER_STATE_OFF;
static uint32_t last_power_change_time = 0;

// --- POWERCUT_Init --- Güvenlik Rölesini başlangıçta KAPALI (Güçsüz) konuma getirir.
void POWERCUT_Init(void)
{
    HAL_GPIO_WritePin(SAFETY_RELAY_GPIO_Port, SAFETY_RELAY_Pin, GPIO_PIN_RESET);
    power_state = POWER_STATE_OFF;
    last_power_change_time = HAL_GetTick();
    shared_data.actuators.power_state = power_state;
}

// --- POWERCUT_SetState ---Röleyi açar (Güç ver) veya kapatır (Güç kes).
// GÜVENLİK: Acil durumdaysa güç açmaya izin vermez.KORUMA: Rölenin çok hızlı açılıp kapanmasını (Chattering) engellemek için zaman kontrolü yapar.
bool POWERCUT_SetState(bool enable_power)
{
    if (shared_data.system.emergency_mode && enable_power) {
        return false; // Acil durumda güç verilmez!
    }
    
    // Röle ömrünü korumak için 100ms bekleme süresi
    if ((HAL_GetTick() - last_power_change_time) < 100) { 
        return false;
    }
    
    HAL_GPIO_WritePin(SAFETY_RELAY_GPIO_Port, SAFETY_RELAY_Pin, 
                      enable_power ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    power_state = enable_power ? POWER_STATE_ON : POWER_STATE_OFF;
    last_power_change_time = HAL_GetTick();
    shared_data.actuators.power_state = power_state;
    return true;
}

// --- POWERCUT_TriggerEmergency --- Anında gücü keser, tüm sistemi Acil Durum Moduna sokar ve Frenleri kilitler.
void POWERCUT_TriggerEmergency(void)
{
    HAL_GPIO_WritePin(SAFETY_RELAY_GPIO_Port, SAFETY_RELAY_Pin, GPIO_PIN_RESET);
    power_state = POWER_STATE_EMERGENCY_OFF;
    last_power_change_time = HAL_GetTick();
    
    shared_data.system.emergency_mode = true;
    shared_data.actuators.power_state = POWER_STATE_EMERGENCY_OFF;
    
    // Frenleri de kilitle!
    BRAKES_EmergencyEngage();
}

































/*#include "pico/stdlib.h"
#include <stdio.h>
#include "../../include/default_pins.h"
#include "../../include/actuators/power_cut.h"

void power_cut_configure_pin()
{
    gpio_init(POWER_CUT_RELAY_DIGITAL);
    gpio_set_dir(POWER_CUT_RELAY_DIGITAL, GPIO_OUT);
}

void set_power_cut_relay(bool status)
{
    gpio_put(POWER_CUT_RELAY_DIGITAL, status);
}
*/
