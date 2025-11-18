/*#ifndef CONTROL_H
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

#endif*/

// Core/Inc/control.h

#ifndef INC_CONTROL_H_
#define INC_CONTROL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

void CONTROL_Init(void);
void CONTROL_Loop(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_CONTROL_H_ */
