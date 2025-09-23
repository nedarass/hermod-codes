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
