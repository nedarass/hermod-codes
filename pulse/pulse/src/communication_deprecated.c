#include "../include/communication_deprecated.h"
#include "../include/default_pins.h"
#include "../include/shared_data.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/sync.h"
#include "hardware/irq.h"
#include <string.h>
#include <stdio.h>

void communication_configure()
{
    // uart baslat
    uart_init(COMM_UART_ID, BAUD_RATE);
    
    // pinleri belirle
    gpio_set_function(COMM_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(COMM_RX_PIN, GPIO_FUNC_UART);

    // fifo
    //uart_set_fifo_enabled(UART_ID, true);
}

//burada problem olabilir
void safe_uart_transfer(mutex_t *mutex, const void *array, size_t element_size) {
    // bufferdan gonderilecek eleman icin yerel kopya al (guvenli gonderim) daha sonra dma ile dene 
    uint8_t temp[element_size];

    buffer_safe_get_latest(mutex, array, temp, element_size);

    // uart ile gonder blockingi dma ile onlemeye calisacaz
    uart_write_blocking(UART_ID, temp, element_size);
}

bool general_comm_transfer_htimer_callback(struct repeating_timer *t)
{
    //volatile func kullan//////////////////////
    safe_uart_transfer(&dht11_mutex, (const void*)&dht11_sensor_buffer, sizeof(dht11_sensor_data_t));
    safe_uart_transfer(&mpu9250_mutex, (const void*)&mpu9250_sensor_buffer, sizeof(mpu9250_sensor_data_t));
    safe_uart_transfer(&optics_mutex, (const void*)&optics_sensor_buffer, sizeof(optics_sensor_data_t));

    return true; //timer tekrar tekrar calisir (true)
}
