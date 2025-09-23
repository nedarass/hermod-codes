//okunan tum sensor verileri shared data da tutulur. 

#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include "sensors/mpu9250.h"
#include "sensors/optics.h"
#include "sensors/navigation.h"
#include "pico/mutex.h"

//sensor verileri kac zaman adimina kadar tutulacak. Zaman adimlarinin gercek periyotlari sensorlere gore degisebilir.
#define BUFFER_SIZE 16

//ivme sensoru MPU9250
extern mutex_t mpu9250_mutex;
extern volatile mpu9250_sensor_data_t mpu9250_sensor_buffer[BUFFER_SIZE];

//optik sensorler
extern mutex_t optics_mutex;
extern volatile optics_sensor_data_t optics_sensor_buffer[BUFFER_SIZE];

//navigation system data
extern mutex_t navigation_mutex;
extern volatile navigation_state_t navigation_buffer[BUFFER_SIZE];


//index arttirmak icin utility
void increase_index(int * last_index);

//tum veriler icin kullanilabilir, verileri gunceller
void safe_update_element(mutex_t *mutex, void *array, size_t index, const void *value, size_t element_size);
void safe_get_element(mutex_t *mutex, const void *array, size_t index, void *destination, size_t element_size);

//veri ensona eklenir ve arkaya gider
void buffer_shift_and_update(mutex_t *mutex, void *array, const void *new_value, size_t element_size);
void buffer_safe_get_latest(mutex_t *mutex, const volatile void *array, void *destination, size_t element_size);


#endif
