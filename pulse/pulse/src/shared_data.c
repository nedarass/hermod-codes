#include "../include/shared_data.h"
#include "hardware/irq.h"
#include "pico/mutex.h"
#include "string.h"


mutex_t mpu9250_mutex;
volatile mpu9250_sensor_data_t mpu9250_sensor_buffer[BUFFER_SIZE] = {0};

mutex_t optics_mutex;
volatile optics_sensor_data_t optics_sensor_buffer[BUFFER_SIZE] = {0};

mutex_t navigation_mutex;
volatile navigation_state_t navigation_buffer[BUFFER_SIZE] = {0};


void increase_index(int * last_index)
{
    *last_index = ((*last_index) + 1) % BUFFER_SIZE;
}

/**
 * @brief bufferlari guvenli degistir
 * @param mutex         sensöre özel mutex
 * 
 * @param array         dizi pointerı
 * @param index         guncellenecek veri noktasi
 * @param value         yeni degeri gosteren pointer
 * @param element_size  ogenin boyutu (byte)
 */
void safe_update_element(mutex_t *mutex, void *array, size_t index, const void *value, size_t element_size) {
    mutex_enter_blocking(mutex);

    memcpy((uint8_t *)array + index * element_size, value, element_size);
    
    mutex_exit(mutex);
}

/**
 * @brief bufferlari guvenli oku
 *
 * @param mutex         okunacak sensör verisine özel mutex kullanilmali (mutex pointer)
 * 
 * @param array         array pointer
 * @param index         ulasilacak element index
 * @param destination   kopyalanacak yere pointer
 * @param element_size  kopyalanan elementin byte boyutu ( dht sensör verisi depolanacaksa sizeof(dht11_sensor_data_t) )
 */
void safe_get_element(mutex_t *mutex, const void *array, size_t index, void *destination, size_t element_size) {
    mutex_enter_blocking(mutex);
    memcpy(destination, (const uint8_t *)array + index * element_size, element_size);
    mutex_exit(mutex);
}




// kaydirma yap yeni veriyi ekle
void buffer_shift_and_update(mutex_t *mutex, void *array, const void *new_value, size_t element_size) {
    mutex_enter_blocking(mutex);

    // dizi referansi saga kaydir
    uint8_t *arr = (uint8_t *)array;
    memmove(arr, arr + element_size, (BUFFER_SIZE - 1) * element_size);

    // en son en yeni
    memcpy(arr + (BUFFER_SIZE - 1) * element_size, new_value, element_size);

    mutex_exit(mutex);
}

// bufferdan en guncel veriyi okur
void buffer_safe_get_latest(mutex_t *mutex, const volatile void *array, void *destination, size_t element_size) {
    mutex_enter_blocking(mutex);

    // buffer_size-1
    memcpy(destination, (const void*)(const volatile  uint8_t *)array + (BUFFER_SIZE - 1) * element_size, element_size);

    mutex_exit(mutex);
}
