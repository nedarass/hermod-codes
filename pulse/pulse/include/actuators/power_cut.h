// Kritik bir durumda veya komutla ana gücü kesmek için kullanılan donanım rölelerini kontrol eder
// power_cut.h - ACİL DURUM GÜNCELLEMESİ
#ifndef POWER_CUT_H
#define POWER_CUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "shared_data.h"

typedef enum {
    POWER_STATE_OFF = 0,
    POWER_STATE_ON = 1,
    POWER_STATE_EMERGENCY_OFF = 2
} PowerState_t;

void POWERCUT_Init(void);
bool POWERCUT_SetState(bool enable_power);
/* Ana güç rölesini kontrol eder.
  enable_power: true  -> Güç ver (Röle Çekili / İletimde) false -> GÜCÜ KES (Röle Bırakılı / Kesik) */
void POWERCUT_TriggerEmergency(void);
/* Yazılımsal olarak gücü anında keser. Genellikle hata durumlarında (Watchdog vb.) çağrılır.*/
PowerState_t POWERCUT_GetState(void);

#ifdef __cplusplus
}
#endif

#endif












/*#ifndef POWER_CUT_H
#define POWER_CUT_H

#include "pico/stdlib.h"
#include "stdio.h"
#include "../default_pins.h"

void power_cut_configure_pin(); //fren erisimi icin gpio pinlerini hazirlar

void set_power_cut_relay(bool status); //fren ciktilarini ayarlar. ornegin gpio x  1 versin vs.

#endif
*/
