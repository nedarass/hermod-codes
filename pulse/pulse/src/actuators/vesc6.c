#include "../../include/actuators/vesc6.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

void vesc6_configure()
{
    // uart baslat
    uart_init(UART_ID, BAUD_RATE);
    
    // pinleri belirle
    gpio_set_function(VESC6_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(VESC6_RX_PIN, GPIO_FUNC_UART);
}

uint8_t calculate_checksum(uint8_t *data, int len) {
    uint8_t checksum = 0;
    for (int i = 1; i < len - 1; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

//test edicez vesc6 protokole internetten baktim
void set_motor_rpm(int rpm)
{
    uint8_t buffer[8];

    buffer[0] = 2;
    buffer[1] = 4; //veri uzunlugu
    buffer[2] = 3; // vesc6 komut kodu (COMM_Set_RPM)
    buffer[3] = (rpm >> 24) & 0xFF;
    buffer[4] = (rpm >> 16) & 0xFF;
    buffer[5] = (rpm >> 8) & 0xFF;
    buffer[6] = rpm & 0xFF;
    buffer[7] = calculate_checksum(buffer, 8);

    uart_write_blocking(UART_ID, buffer, 8);
}
