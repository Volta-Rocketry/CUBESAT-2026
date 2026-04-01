#pragma once

#include <stdint.h>

// Baud Rate
#define BAUD_RATE 115200

// MPU PIN DEFINITIONS
#define MPU_SCK 36
#define MPU_MOSI 35
#define MPU_MISO 37
#define MPU_CS 47

// BME PIN DEFINITIONS
#define BME_SDA 4
#define BME_SCL 5

// BNO PIN DEFINITIONS
#define BNO_SCK 12
#define BNO_MOSI 11
#define BNO_MISO 13
#define BNO_CS 14

// UBLOX PIN DEFINITIONS
#define UBLOX_TX 18
#define UBLOX_RX 17

// TRANSDUCER PIN DEFINITION
#define TRANSDUCER_PIN 16

// ACTUATOR PIN DEFINITION
#define ACTUATOR_PIN 15

// LED PIN DEFINITIONS
#define LED_RED_PIN 38
#define LED_GREEN_PIN 39
#define LED_BLUE_PIN 40

// BUZZER PIN DEFINITIONS
#define BUZZER_PIN 2




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
    float timestamp;   // Timestamp
    uint32_t timeGPS;  // GPS time
    uint32_t dateGPS;  // GPS date
    double lat;        // Latitude
    double lon;        // Longitude
    float alt;         // Altitude above mean sea level
    float speed;       // Speed
    float course;      // Course over ground
    uint8_t satellites;// Number of satellites in view
    float hdop;        // Horizontal Dilution of Precision
    bool valid;        // Validity of the GPS data
};

struct StructTransducers {
    float timestamp;  // Timestamp
    float transducer; // Transducer reading
};
