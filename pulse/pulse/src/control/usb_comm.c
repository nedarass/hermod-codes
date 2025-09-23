#include "../../include/control/usb_comm.h"
#include "../../include/control/commands.h"
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include <stdio.h>
#include <string.h>

static ring_buffer_t rx_buffer;
static repeating_timer_t timer;
static uint8_t dummy_data = 42;

void usb_comm_init(void) {
    stdio_init_all();
    ring_buffer_init(&rx_buffer);
    
    // Initialize hardware timer for periodic data transmission
    add_repeating_timer_ms(SEND_INTERVAL_MS, usb_comm_timer_callback, NULL, &timer);
}

bool usb_comm_timer_callback(repeating_timer_t *t) {
    // Send dummy data periodically
    usb_comm_send_data(dummy_data);
    return true;
}

void usb_comm_send_data(uint8_t data) {
    printf("DATA:%02X\n", data);
}

void usb_comm_process(void) {
    // Read available data from USB
    int ch = getchar_timeout_us(0);
    while (ch != PICO_ERROR_TIMEOUT) {
        ring_buffer_push(&rx_buffer, (uint8_t)ch);
        ch = getchar_timeout_us(0);
    }
    
    // Check if we have enough data for a command
    if (ring_buffer_get_count(&rx_buffer) >= COMMAND_SIZE) {
        uint8_t command_buffer[COMMAND_SIZE];
        
        // Peek at the data without removing it
        if (ring_buffer_peek_range(&rx_buffer, command_buffer, 0, COMMAND_SIZE)) {
            // Process the command
            if (process_command(command_buffer)) {
                // Command was valid, remove it from buffer
                ring_buffer_discard(&rx_buffer, COMMAND_SIZE);
            } else {
                // Invalid command, discard first byte and try again
                uint8_t dummy;
                ring_buffer_pop(&rx_buffer, &dummy);
            }
        }
    }
}
