#include "../../include/actuators/inverter_rs485.h"

void rs485_modbus_configure()
{
    // uart baslat
    uart_init(UART_ID, BAUD_RATE);
    
    // pinleri belirle
    gpio_set_function(RS485_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(RS485_RX_PIN, GPIO_FUNC_UART);
    
    // rs485 yönlendirme pini çıkış olarak ayarlanıyor
    gpio_init(RS485_DE_PIN);
    gpio_set_dir(RS485_DE_PIN, GPIO_OUT);

    gpio_put(RS485_DE_PIN, 1);  // de yuksek = tx aktif
    sleep_us(RS485_DELAY_US);   // mod gecisi icin transceiver bekliyoruz


    // modbus icin ayarmis
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);

    // fifo
    //uart_set_fifo_enabled(UART_ID, true);
}

void rs485_write(uint8_t *message, uint8_t len)
{
    // veriyi yaz
    uart_puts(UART_ID, message);
    
    // veri gonderimini bekle
    while (!uart_is_writable(UART_ID)) {
        tight_loop_contents();
    }
    sleep_ms(RS485_DELAY_US);
}
