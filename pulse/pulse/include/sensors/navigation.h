#ifndef NAVIGATION_H
#define NAVIGATION_H

#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "mpu9250.h"
#include "encoder_read.h"

// Navigation state structure
typedef struct {
    uint32_t timestamp;           // Timestamp in microseconds
    
    // Position, velocity, acceleration in the trajectory direction
    float position;               // Position in meters
    float velocity;               // Velocity in m/s
    float acceleration;           // Acceleration in m/s^2
    
    // Raw sensor values (for diagnostics)
    int32_t encoder_count;        // Raw encoder count
    float accel_raw;              // Raw acceleration along trajectory
} navigation_state_t;

// Kalman filter state
typedef struct {
    // State vector [position, velocity, acceleration]
    float x[3];
    
    // State covariance matrix (3x3)
    float P[3][3];
    
    // Process noise covariance (3x3)
    float Q[3][3];
    
    // Measurement noise covariance (2x2) - for encoder and accelerometer
    float R[2][2];
    
    // Last update timestamp
    uint32_t last_update_time;
    
    // Calibration values
    float encoder_pulses_per_meter;
} kalman_filter_t;

// Initialize the navigation system and Kalman filter
void navigation_init(float encoder_pulses_per_meter, float process_noise, float encoder_noise, float accel_noise);

// Update navigation state with new sensor readings
void navigation_update(uint32_t timestamp, int32_t encoder_count, float accel);

// Get the current navigation state
void navigation_get_state(navigation_state_t *state);

// Reset the Kalman filter state
void navigation_reset(void);

// Set Kalman filter parameters
void navigation_set_params(float process_noise, float encoder_noise, float accel_noise);

#endif /* NAVIGATION_H */
