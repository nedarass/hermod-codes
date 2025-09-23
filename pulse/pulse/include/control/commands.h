#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdint.h>
#include <stdbool.h>

#define CMD_OPEN_LED  0x01
#define CMD_CLOSE_LED 0x02

typedef struct {
    uint8_t command_id;
    uint8_t payload[15];
} command_t;

bool process_command(uint8_t *data);
void execute_open_led(void);
void execute_close_led(void);

#endif
