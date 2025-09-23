#include "../../include/sensors/mpu9250.h"
#include "../../include/default_pins.h"
#include "../../include/shared_data.h"
#include "hardware/i2c.h"

//ortalama 40ms okuma suresi (biraz abartarak hesapladim)
bool read_mpu9250(mpu9250_sensor_data_t* data)
{
    uint8_t buffer[MPU9250_READ_LENGTH];
    uint8_t reg = MPU9250_ACCEL_XOUT_H;
    
    // okuma baslangici adresini ver &reg
    int ret = i2c_write_blocking(I2C_PORT, MPU9250_ADDRESS, &reg, 1, true);
    if(ret < 0) {
        return false;
    }
    
    // oku
    ret = i2c_read_blocking(I2C_PORT, MPU9250_ADDRESS, buffer, MPU9250_READ_LENGTH, false);
    if(ret < 0) {
        return false;
    }
    
    // bigendian - 16 signed int convert
    data->accel_x = (int16_t)((buffer[0] << 8) | buffer[1]);
    data->accel_y = (int16_t)((buffer[2] << 8) | buffer[3]);
    data->accel_z = (int16_t)((buffer[4] << 8) | buffer[5]);
    data->temp    = (int16_t)((buffer[6] << 8) | buffer[7]);
    data->gyro_x  = (int16_t)((buffer[8] << 8) | buffer[9]);
    data->gyro_y  = (int16_t)((buffer[10] << 8) | buffer[11]);
    data->gyro_z  = (int16_t)((buffer[12] << 8) | buffer[13]);
    
    return true;
}

bool mpu9250_htimer_callback(struct repeating_timer *t)
{
    mpu9250_sensor_data_t sensor_data;

    bool is_read_successfull = read_mpu9250(&sensor_data); //timestamp harici sensor_data doldur

    if (is_read_successfull)
    {
        //basarili
        sensor_data.timestamp = time_us_32();
        buffer_shift_and_update(&mpu9250_mutex, (uint8_t*) mpu9250_sensor_buffer, &sensor_data, sizeof(mpu9250_sensor_data_t));
    }
    else
    {
        //basarisiz
    }

    return true; //timer tekrar tekrar calisir (true)
}
