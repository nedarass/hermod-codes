// pulse/pulse/src/actuators/cooling.c
#include "cooling.h" 
#include "shared_data.h"

/* --- DİKKAT: CUBEMX AYARLARI ---
 * Fanları kontrol eden Röle veya MOSFET pinine: "FAN_RELAY"
 * etiketini (User Label) veriniz.
 */

// Varsayılan Pin Tanımı (Güvenlik)
#ifndef FAN_RELAY_Pin
    // Tanımlı değilse kodun patlamasını önle ama çalışmaz.
#endif

void COOLING_Init(void)
{
    // Başlangıçta fanlar KAPALI olsun (Enerji tasarrufu)
    COOLING_SetFanState(false);
}

void COOLING_SetFanState(bool state)
{
    #ifdef FAN_RELAY_Pin
        // Röle veya MOSFET'i tetikle
        // Genelde: HIGH = Açık, LOW = Kapalı
        HAL_GPIO_WritePin(FAN_RELAY_GPIO_Port, FAN_RELAY_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
    #endif
    
    // Durumu Shared Data'ya işle (Yer istasyonu görsün)
    shared_data.actuators.cooling_state = state ? 1 : 0;
}
