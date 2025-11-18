#ifndef OPTICS_H
#define OPTICS_H

#include "pico/stdlib.h"
#include "hardware/timer.h"
#include <stdbool.h>


typedef struct {
    uint32_t timestamp;   // okuma zaman adimi  us
    bool transition_state;
} optics_sensor_data_t;

void pio_interrupt_handler();



#endif
/*HAL GPIO EXTI: Omron veya Optik sensörler için STM32 Harici Kesme (EXTI)
kullanılmalıdır. Bu, Omron algılandığında anında tepki vermenin doğru yoludur.*/

