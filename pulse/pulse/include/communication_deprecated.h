#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

#include "sensors/dht11.h"
#include "sensors/mpu9250.h"
#include "sensors/optics.h"
#include "pico/mutex.h"

void communication_configure(); //pico pi5 arasi iletisim icin konfigurasyon

void safe_uart_transfer(mutex_t *mutex, const void *array, size_t element_size); //uart veri transferi

//pico pi arasi veri alisverisi icin donanimsal zamanlayici callbacki
bool general_comm_transfer_htimer_callback(struct repeating_timer *t); //

#endif
