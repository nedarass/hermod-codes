#include "../../include/control/communication.h"
#include "../../include/control/control.h"
#include "tusb.h"
#include "tusb_config.h"
#include <string.h>

#define BUFFER_SIZE 128

volatile uint8_t usb_buffer[BUFFER_SIZE];  // gelen veriler burada.

void buffer_clear()
{
    memset((void*)usb_buffer, 0, BUFFER_SIZE);
}

int read_protocol_data()
{
    if(!tud_cdc_available())
    {
        return 0;
    }

    int bytes_read = (int)tud_cdc_read((void *)usb_buffer, BUFFER_SIZE - 1);
    if (bytes_read > 0) {
        usb_buffer[bytes_read] = '\0';  // null terminasyonu yap cstr
    }

    return bytes_read;  // okunan byte sayisi
}

void write_protocol_data(uint8_t* data, int size)
{
    if (size > 0) {
        tud_cdc_write(data, size);
        tud_cdc_write_flush();  // tusb ic bufferda tamamlanmayan kismi aninda gonderir
    }
}
