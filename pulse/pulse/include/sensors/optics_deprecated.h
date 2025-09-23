#ifndef OPTICS_H_
#define OPTICS_H_

#include "pico/stdlib.h"
#include "hardware/timer.h"
#include <stdbool.h>

#define NUM_SAMPLES_PER_CHANNEL 8

extern volatile uint16_t dma_buffer[NUM_SAMPLES_PER_CHANNEL * 2]; //iki adet optik sensör var 

typedef struct {
    uint32_t timestamp;   // okuma zaman adimi  us
    float intensity0;
    float intensity1;
} optics_sensor_data_t;


void optics_configure_dma_channel();

bool read_optics(optics_sensor_data_t* humidity);

bool optics_htimer_callback(struct repeating_timer *t);


#endif
