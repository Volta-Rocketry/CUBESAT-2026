#pragma once

#include <stdint.h>

#define LED_PIN 2
#define BLINK_S 0.5
// Baud Rate
#define BAUD_RATE 115200


struct StructMPU9250{
    float timestamp;  // Timestamp
    // Acceleration
    float MPU_ax;     // MPU9250 acceleration on the X axis
    float MPU_ay;     // MPU9250 acceleration on the Y axis
    float MPU_az;     // MPU9250 acceleration on the Z axis
    // Angular velocity
    float MPU_gx;     // MPU9250 angular velocity on the X axis
    float MPU_gy;     // MPU9250 angular velocity on the Y axis
    float MPU_gz;     // MPU9250 angular velocity on the Z axis
    // Magnetic field
    float MPU_mx;     // MPU9250 magnetic field on the X axis
    float MPU_my;     // MPU9250 magnetic field on the Y axis
    float MPU_mz;     // MPU9250 magnetic field on the Z axis
};

struct StructBNO055{
    float timestamp;  // Timestamp
    // Acceleration
    float BNO_ax;     // BNO055 acceleration on the X axis
    float BNO_ay;     // BNO055 acceleration on the Y axis
    float BNO_az;     // BNO055 acceleration on the Z axis
    // Angular velocity
    float BNO_gx;     // BNO055 angular velocity on the X axis
    float BNO_gy;     // BNO055 angular velocity on the Y axis
    float BNO_gz;     // BNO055 angular velocity on the Z axis
    // Magnetic field
    float BNO_mx;     // BNO055 magnetic field on the X axis
    float BNO_my;     // BNO055 magnetic field on the Y axis
    float BNO_mz;     // BNO055 magnetic field on the Z axis
};

struct StructBME280 {
    float temp;       // Temperature
    float humidity;   // Humidity
    float pressure;   // Pressure
    float altitude;   // Altitude
    float timestamp;  // Timestamp
};

struct StructUblox {
    float timestamp;  // Timestamp
    float lat;        // Latitude
    float lon;        // Longitude
    float alt;        // Altitude
    float speed;      // Speed
};

