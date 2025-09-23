#ifndef MPU9250_H
#define MPU9250_H

#include "pico/stdlib.h"
#include "hardware/timer.h"

//pointer depola ileride
//mpu icin veri tutma structi
typedef struct {
    uint32_t timestamp;   // okuma zaman adimi  us
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t temp;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
} mpu9250_sensor_data_t;

//mpu verisini okur ve sensor data structina yazar.
bool read_mpu9250(mpu9250_sensor_data_t* sensor_data);

//mpu okumasi icin donanimsal zamanlayici callback
bool mpu9250_htimer_callback(struct repeating_timer *t);


#endif

/*

#define I2C_PORT          i2c0
#define I2C_BAUD          (400 * 1000)
#define I2C_SDA_PIN       4
#define I2C_SCL_PIN       5

#define MPU_ADDR          0x68
#define AK8963_ADDR       0x0C

// MPU-9250 registers
#define REG_PWR_MGMT_1    0x6B
#define REG_CONFIG        0x1A
#define REG_GYRO_CFG      0x1B
#define REG_ACCEL_CFG     0x1C
#define REG_INT_PIN_CFG   0x37
#define REG_INT_ENABLE    0x38
#define REG_ACCEL_XOUT_H  0x3B   // first of 14 bytes accel+gyro+temp

// Data-ready flag
static volatile bool mpu_drdy = false;

// ───── Low-level I²C helpers ───────────────────────────────────────────────────
static void i2c_write_byte(uint8_t reg, uint8_t data) {
    uint8_t buf[2] = {reg, data};
    i2c_write_blocking(I2C_PORT, MPU_ADDR, buf, 2, false);
}

static void i2c_read_bytes(uint8_t reg, uint8_t *dst, size_t n) {
    i2c_write_blocking(I2C_PORT, MPU_ADDR, &reg, 1, true);
    i2c_read_blocking (I2C_PORT, MPU_ADDR, dst,  n, false);
}

// ───── GPIO interrupt handler ─────────────────────────────────────────────────
static void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == 16 && (events & GPIO_IRQ_EDGE_RISE))
        mpu_drdy = true;
}

// ───── Sensor init ────────────────────────────────────────────────────────────
static void mpu_init(void) {
    sleep_ms(100);
    i2c_write_byte(REG_PWR_MGMT_1, 0x01);      // clock = PLL
    i2c_write_byte(REG_CONFIG,     0x03);      // DLPF 44 Hz
    i2c_write_byte(REG_GYRO_CFG,   0x00);      // ±250 dps
    i2c_write_byte(REG_ACCEL_CFG,  0x00);      // ±2 g

    // Interrupt: active-high, push-pull, latched until status cleared
    i2c_write_byte(REG_INT_PIN_CFG, 0x10);
    i2c_write_byte(REG_INT_ENABLE,  0x01);     // Data Ready
}


// I²C
    i2c_init(I2C_PORT, I2C_BAUD);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    // INT pin
    gpio_init(16);
    gpio_set_dir(16, GPIO_IN);
    gpio_pull_down(16);                       // ensure defined state
    gpio_set_irq_enabled_with_callback(16, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    mpu_init();

    uint8_t buf[14];
    while (true) {
        if (mpu_drdy) {
            mpu_drdy = false;

            i2c_read_bytes(REG_ACCEL_XOUT_H, buf, 14);

            int16_t ax = (buf[0]  << 8) | buf[1];
            int16_t ay = (buf[2]  << 8) | buf[3];
            int16_t az = (buf[4]  << 8) | buf[5];
            int16_t temp_raw = (buf[6]  << 8) | buf[7];
            int16_t gx = (buf[8]  << 8) | buf[9];
            int16_t gy = (buf[10] << 8) | buf[11];
            int16_t gz = (buf[12] << 8) | buf[13];

            // Convert to physical units if required; raw values shown here.
            printf("A: %d %d %d  G: %d %d %d  T: %d\n", ax, ay, az, gx, gy, gz, temp_raw);
        }
        tight_loop_contents();
    }
 


*/
