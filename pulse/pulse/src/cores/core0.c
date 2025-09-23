//high freq islemler
//high freq sensor ve yonergeler icin timer atanir.

#include "pico/stdlib.h"
#include "pico/time.h"

#include "../../include/cores/core0.h"

#include "../../include/sensors/optics.h"

#include "../../include/control/communication.h"
#include "../../include/control/control.h"
#include "tusb.h"
#include "hardware/structs/scb.h"

void core0_entry()
{

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);


    //printf("Pico USB Serial Initialized!\n");

    bool hey = false;

    while (true) {
        buffer_clear();
        int size = read_protocol_data();  // veriyi oku ve boyutunuu al
        
        if (size > 0) {
            write_protocol_data(usb_buffer, size);

            hey=!hey;
            gpio_put(PICO_DEFAULT_LED_PIN, hey);
        }
    }
}
