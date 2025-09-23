#ifndef USB_COMM_H
#define USB_COMM_H

#include <stdint.h>
#include <stdbool.h>
#include "pico/time.h"  // Include this for repeating_timer definition
#include "ring_buffer.h"

#define COMMAND_SIZE 16  // 128 bits = 16 bytes
#define SEND_INTERVAL_MS 4000  // Send data every 100ms

void usb_comm_init(void);
void usb_comm_send_data(uint8_t data);
void usb_comm_process(void);
bool usb_comm_timer_callback(repeating_timer_t *t);

#endif
