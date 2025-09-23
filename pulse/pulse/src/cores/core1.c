//low freq az onemli islemler

#include "pico/stdlib.h"
#include "pico/time.h"

#include "../../include/cores/core1.h"

#include "../../include/sensors/mpu9250.h"

#include "../../include/control/communication.h"
#include "../../include/control/control.h"

#include <stdio.h>

//for sensor_readings

void core1_entry()
{
    // interruptlar engellenmez
    while (true) {
        tight_loop_contents();
    }
}
