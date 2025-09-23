#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "pico/stdlib.h"
#include "hardware/timer.h"

extern volatile uint8_t usb_buffer[];  // buffer

// USB veri alındığında çağrılan callback fonksiyonu
void buffer_clear();

int read_protocol_data();

void write_protocol_data(uint8_t* data, int size);

#endif
