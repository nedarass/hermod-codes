#ifndef CONTROL_H
#define CONTROL_H

#include "pico/stdlib.h"
#include "hardware/timer.h"
#include <stdio.h>
#include "pico/stdlib.h"

enum MessageType 
{
    PING_PONG = 0,
    REQUEST = 1,
    WRITE = 2
};

int get_message_type_non_sdk(int8_t * data, int size);

#endif
