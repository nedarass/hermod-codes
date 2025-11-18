#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/irq.h"
#include "../../include/sensors/optics.h"
#include "../../include/default_pins.h"

void pio_interrupt_handler() {
    // pio fifosunu oku
    uint32_t gpio_value = pio_sm_get_rx_fifo(pio0, 0);

    // kesme temzile
    irq_clear(OPTICS_INTERRUPT_IRQ);
}
/*// Core/Src/optics_driver.c

#include "main.h"
#include "shared_data.h"
#include <stdio.h>

// Global sistem durum yapısı
extern System_State_t g_system_state;

// Optik/Omron sensörleri genellikle kritik noktalarda EXTI olarak kullanılır.
// Varsayım: Omron1 ve Omron2 pinleri tanımlı (CubeMX'te)

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    // Bu fonksiyon, main.c'deki HAL_GPIO_EXTI_Callback'in yerini alır.

    // OPTICS/OMRON PINLERİNİNİZİ BURADA TANIMLAYIN (Örn: Omron1_Pin)
    // Bu pinler main.h'de olmalıdır.

    if (GPIO_Pin == Omron1_Pin)
    {
        // Kritik bir limit switch veya başlangıç/bitiş işareti
        g_system_state.error_flags |= (1 << 30); // Örnek: Özel bir Omron Bayrağı
        printf("KRITIK: Omron 1 Algilandi!\r\n");
        // Acil Fren Komutu tetiklenebilir
        // COMMAND_ProcessIncoming(CMD_BRAKE_ACTUATE, TYPE_U8, (uint8_t[]){100}, 1);
    }

    if (GPIO_Pin == Omron2_Pin)
    {
        // Pozisyon sıfırlama işareti
        g_system_state.position_m = 0.0f;
        printf("Pozisyon Sifirlandi.\r\n");
    }
}*/
