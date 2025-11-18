/* Basitleştirme: Ring buffer mekanizması telemetri için gereksizdir.
Merkezi bir tek okuma (g_system_state yapısı) tutulmalıdır. Mutex'ler
STM32'nin RTOS Mutex'leri veya basit kesme engelleme (__disable_irq()) ile değiştirilmelidir */
/*
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


#endif*/

/* // Core/Inc/shared_data.h

#ifndef INC_SHARED_DATA_H_
#define INC_SHARED_DATA_H_

#include "main.h" // HAL tipleri için
#include <stdint.h>

// Tüm sistem durumunu tutan merkezi yapı
typedef struct {
    // SENSÖR VE MONİTÖR VERİLERİ (Monitoring Page)
    float velocity_mps;         // ID_VELOCITY
    float acceleration_mss;     // ID_ACCELERATION
    float position_m;           // ID_POSITION
    uint16_t raw_voltage_mv;    // ID_VOLTAGE (mV)
    int16_t raw_current_ma;     // ID_CURRENT (mA)
    float power_w;              // ID_POWER
    int16_t raw_temp_ntc1;      // ID_TEMPERATURE_NTC1 (Santigrat * 100)
    uint8_t brake_status;       // ID_BRAKE_STATUS (0: Açık, 1: Kapalı/Aktif)

    // SİSTEM SAĞLIK VERİLERİ (Health Check Page)
    float cpu_temp_c;           // ID_CPU_TEMP
    uint32_t error_flags;       // ID_ERROR_FLAG (Hata kodları)
    uint16_t ping_ms;           // ID_PING_RESPONSE (ms)
    uint8_t power_line_status;  // ID_POWER_LINE_STATUS (0: OFF, 1: ON)
    uint8_t rtos_task_status;   // ID_RTOS_STATUS (0: OK)

} System_State_t;

// Global Sistem Durumu (Yalnızca bir kez shared_data.c'de tanımlanır)
extern System_State_t g_system_state;


// Hata Bayrakları Bit Tanımları (Bitmask olarak)
#define ERR_FLAG_MPU_FAIL       (1 << 0)
#define ERR_FLAG_NTC_OOR        (1 << 1)
#define ERR_FLAG_COMM_TIMEOUT   (1 << 2)
#define ERR_FLAG_POWER_TRIP     (1 << 3)

#endif /* INC_SHARED_DATA_H_ */*/


#endif
