//Tum pinler buradan ayarlanabilir !!

#ifndef DEFAULT_PINS_H
#define DEFAULT_PINS_H

#define DHT11_PIN 4

#define I2C_PORT i2c0
#define MPU9250_ADDRESS 0x68
#define MPU9250_ACCEL_XOUT_H 0x3B
#define MPU9250_READ_LENGTH 14

#define OPTICS_0_PIN 26
#define OPTICS_1_PIN 27
#define OPTICS_INTERRUPT_IRQ 0



//actuators

//brakes
#define BRAKE_0_DIGITAL 10
#define BRAKE_1_DIGITAL 11
#define BRAKE_2_DIGITAL 12
#define BRAKE_3_DIGITAL 13

#define BLINKER_DIGITAL 14
#define POWER_CUT_RELAY_DIGITAL 15

//inverter with rs485
#define UART_ID uart0
#define BAUD_RATE 250000 // bu iyi gibi geldi ama bizim devreler ryakin belki arttirabiliriz. her byte 40us suan
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE

// uart tx ve rx
#define RS485_TX_PIN 0
#define RS485_RX_PIN 1

// vesc6 tx ve rx
#define VESC6_TX_PIN 8
#define VESC6_RX_PIN 9

// rs485de pini transceiver kontrolu icin
// bu pini, rs485 de veya re girisine baglaacaz. okuma mi yazma mi yaptigimizi soyluyor hgh=yazma, low=okuma
#define RS485_DE_PIN 2

// transceiver gecisi okuma7yazma
#define RS485_DELAY_US 1



//dc motors
#define MOTOR0_PIN 2
#define MOTOR1_PIN 3
#define MOTOR2_PIN 4
#define MOTOR3_PIN 5




//communication rpi5 - rpi5 ile iletisimde kullanilacak port 
#define COMM_UART_ID uart1
#define BAUD_RATE 250000
#define COMM_TX_PIN 4
#define COMM_RX_PIN 5


#endif
