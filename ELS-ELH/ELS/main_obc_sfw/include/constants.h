#pragma once

#include <stdint.h>


// Baud Rate
#define BAUD_RATE 115200

// GPS Baud Rate
#define GPS_BAUD 9600

// MPU PIN DEFINITIONS
#define MPU_SCK 16
#define MPU_MOSI 17
#define MPU_MISO 27
#define MPU_CS 4

// BNO PIN DEFINITIONS
#define BNO_SDA 21
#define BNO_SCL 22

// BME PIN DEFINITIONS
#define BME_SCK 18
#define BME_MOSI 23
#define BME_MISO 19
#define BME_CS 33

// UBLOX PIN DEFINITIONS
#define UBLOX_TX 32
#define UBLOX_RX 36

// FLASH PIN DEFINITIONS
#define FLASH_SCK 18
#define FLASH_MOSI 23
#define FLASH_MISO 19 
#define FLASH_CS 15

// SD PIN DEFINITIONS
#define SD_SCK 16
#define SD_MOSI 17
#define SD_MISO 27
#define SD_CS  3// esta definido en el expansor

// TRANSDUCER PIN DEFINITION
#define TRANSDUCER_PIN 16

// ACTUATOR PIN DEFINITION
#define ACTUATOR_PIN 15

// LED PIN DEFINITIONS
// Estan en el expansor
#define LED_RED_PIN 5
#define LED_GREEN_PIN 6
#define LED_BLUE_PIN 7

// BUZZER PIN DEFINITIONS
// Esta definido en el expansor
#define BUZZER_PIN 4

// flight computer definitions
#define PIN_BUTTON 34
#define PIN_CS_SD SD_CS
#define LAUNCH_ACCEL_THRESHOLD_MS2 29.43f  // cambiar de 3g a una menor que pueda acelear el dron de prueba
#define FAST_SAMPLE_INTERVAL_MS 10
#define SLOW_SAMPLE_INTERVAL_MS 1000

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
    // Chip temperature
    float MPU_temp;   // MPU9250 chip temperature
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
    // Quaternions
    float BNO_qw;     // BNO055 quaternion W component
    float BNO_qx;     // BNO055 quaternion X component
    float BNO_qy;     // BNO055 quaternion Y component
    float BNO_qz;     // BNO055 quaternion Z component
};

struct StructBME280 {
    float timestamp;  // Timestamp
    float temp;       // Temperature
    float humidity;   // Humidity
    float pressure;   // Pressure
    float altitude;   // Altitude
};

struct StructUblox {
    float timestamp;   // Timestamp

    // GPS time
    uint8_t hour;      // GPS hour
    uint8_t minute;    // GPS minute
    uint8_t second;    // GPS second

    // GPS date
    uint16_t year;     // GPS year
    uint8_t month;     // GPS month
    uint8_t day;       // GPS day

    double latitude;   // Latitude
    double longitude;  // Longitude
    float altitude;    // Altitude above mean sea level
    float speed;       // Speed
    float course;      // Course over ground
    uint8_t satellites;// Number of satellites in view
    float hdop;        // Horizontal Dilution of Precision
    bool valid;        // Validity of the GPS data
};

struct StructTransducer {
    float timestamp;  // Timestamp
    float voltage;    // Transducer reading
    float pressureTransducer;   // Transducer reading
};


// Calibration data structure
struct CalibrationDataMPU {
    // MPU9250 calibration data
    float mpuGyroBiasX;
    float mpuGyroBiasY;
    float mpuGyroBiasZ;
    float mpuAccBiasX;
    float mpuAccBiasY;
    float mpuAccBiasZ;

    // Temperature errors
    float tempRef;
    float gyroTCO;
    float accTCO;

    // Hard iron offsets
    float mpuMagOffsetX;
    float mpuMagOffsetY;
    float mpuMagOffsetZ;

    // Soft iron
    float mpuMagScaleX;
    float mpuMagScaleY;
    float mpuMagScaleZ;
};

struct CalibrationDataBNO {
    // BNO055 calibration data
    uint8_t bnoSystemStatus;
    uint8_t bnoGyroStatus;
    uint8_t bnoAccStatus;
    uint8_t bnoMagStatus;
};

struct CalibrationDataBME {
    // BME280 calibration data
    float bmePresRef;  // Pressure reference for altitude calculation

};