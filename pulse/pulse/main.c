/*

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/dma.h"
#include "hardware/timer.h"
#include "hardware/uart.h"
#include "pico/multicore.h"
#include "tusb.h"
#include "hardware/structs/scb.h"  // For __WFE()



#include "include/cores/core0.h"
#include "include/cores/core1.h"

#include "include/control/communication.h"
#include "include/control/control.h"

#include "include/sensors/encoder_read.h"

#include "include/sensors/mpu9250.h"
#include "include/shared_data.h"

#include "include/sensors/m365_register_map.h"
#include "include/sensors/ninebot.h"



int main(void)
{
    // Initialize standard I/O
    stdio_init_all();
    sleep_ms(200);                              


    uart_init(uart0, 115200);

    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);

    while (1)
    {
        const char *msg = "HELLO\r\n";
        uart_write_blocking(uart0, (const uint8_t *)msg, 7);

        sleep_ms(1000);
    }
    

    while (1) __wfi();    // sonsuz bekleme

        
    // This should never be reached
    return 0;
}

*/

/*
#include "pico/stdlib.h"
#include "hardware/uart.h"

#define BLINK_PIN 25  // Dahili LED (veya başka bir GPIO kullanabilirsin)

int main() {
    stdio_init_all();

    // UART0 başlat (TX=GP0, RX=GP1)
    uart_init(uart0, 115200);
    gpio_set_function(0, GPIO_FUNC_UART);  // TX
    gpio_set_function(1, GPIO_FUNC_UART);  // RX

    // Blink pini çıkış olarak ayarla
    gpio_init(BLINK_PIN);
    gpio_set_dir(BLINK_PIN, GPIO_OUT);

    while (true) {
        uart_puts(uart0, "Test mesajı\n");

        // Blink tetikleme
        gpio_put(BLINK_PIN, 1);
        sleep_ms(50); // kısa yanma süresi
        gpio_put(BLINK_PIN, 0);

        sleep_ms(1000);
    }
}


*/


#include "pico/stdlib.h"
#include "include/control/usb_comm.h"
#include <stdio.h>

int main() {
    // Initialize USB communication
    usb_comm_init();
    
    // Wait for USB connection
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    
    printf("USB Communication System Started\n");
    
    // Main loop
    while (1) {
        usb_comm_process();
        sleep_ms(1);  // Small delay to prevent CPU overload
    }
    
    return 0;
}
