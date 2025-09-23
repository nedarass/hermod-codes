#include "../../include/control/commands.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

// LED is on GPIO 25 (onboard LED)
#define LED_PIN 25

static bool led_initialized = false;

static void init_led(void) {
    if (!led_initialized) {
        gpio_init(LED_PIN);
        gpio_set_dir(LED_PIN, GPIO_OUT);
        led_initialized = true;
    }
}

bool process_command(uint8_t *data) {
    command_t *cmd = (command_t *)data;
    
    // Simple command validation (you can add checksum here)
    switch (cmd->command_id) {
        case CMD_OPEN_LED:
            execute_open_led();
            return true;
            
        case CMD_CLOSE_LED:
            execute_close_led();
            return true;
            
        default:
            // Unknown command
            return false;
    }
}

void execute_open_led(void) {
    init_led();
    gpio_put(LED_PIN, 1);
    printf("LED: ON\n");
}

void execute_close_led(void) {
    init_led();
    gpio_put(LED_PIN, 0);
    printf("LED: OFF\n");
}
