#pragma once

#include <stdint.h>


// Baud Rate
#define BAUD_RATE 115200

// GPS Baud Rate
#define GPS_BAUD 9600

// GY87 PIN DEFINITIONS
#define GY_SCL 22
#define GY_SDA 21

// BNO PIN DEFINITIONS
#define BNO_SDA 21
#define BNO_SCL 22

// BME PIN DEFINITIONS
#define BME_SCK 18
#define BME_MOSI 23
#define BME_MISO 19
#define BME_CS 26

// UBLOX PIN DEFINITIONS
#define UBLOX_TX 25
#define UBLOX_RX 36

// FLASH PIN DEFINITIONS
#define FLASH_SCK 18
#define FLASH_MOSI 23
#define FLASH_MISO 19 
#define FLASH_CS 5

// SD PIN DEFINITIONS
#define SD_SCK 18
#define SD_MOSI 22
#define SD_MISO 19
#define SD_CS  2

// ACTUATOR PIN DEFINITION
#define ACTUATOR1_PIN 16
#define ACTUATOR2_PIN 4

// LED PIN DEFINITIONS
#define LED_RED_PIN 13
#define LED_GREEN_PIN 14
#define LED_BLUE_PIN 27

// BUZZER PIN DEFINITIONS
#define BUZZER_PIN 17

// PWM DEFINITIONS
#define RED_CHANNEL 0
#define GREEN_CHANNEL 1
#define BLUE_CHANNEL 2
#define BUZZER_CHANNEL 3

#define PWM_FREQ 5000
#define PWM_RESOLUTION 8

// flight computer definitions
#define LAUNCH_ACCEL_THRESHOLD_MS2 29.43f  // cambiar de 3g a una menor que pueda acelear el dron de prueba
#define FAST_SAMPLE_INTERVAL_MS 10
#define SLOW_SAMPLE_INTERVAL_MS 1000

// FLASH
#define FLASH_PAGE_SIZE 256UL
#define FLASH_TOTAL_BYTES 16777216UL

#define CMD_WRITE_ENABLE 0x06
#define CMD_CHIP_ERASE 0xC7
#define CMD_PAGE_PROGRAM 0x02
#define CMD_READ_DATA 0x03
#define CMD_READ_STATUS 0x05
#define CMD_JEDEC_ID 0x9F



struct StructMPU6050{
    uint32_t timestamp;  // Timestamp
    // Acceleration
    float MPU_ax;     // MPU9250 acceleration on the X axis
    float MPU_ay;     // MPU9250 acceleration on the Y axis
    float MPU_az;     // MPU9250 acceleration on the Z axis
    // Angular velocity
    float MPU_gx;     // MPU9250 angular velocity on the X axis
    float MPU_gy;     // MPU9250 angular velocity on the Y axis
    float MPU_gz;     // MPU9250 angular velocity on the Z axis
    // Chip temperature
    float MPU_temp;   // MPU9250 chip temperature
};

struct StructQMC5883L{
    uint32_t timestamp; // Timestamp
    float QMC_mx;       // QMC5883L magnetic field on the X axis
    float QMC_my;       // QMC5883L magnetic field on the Y axis
    float QMC_mz;       // QMC5883L magnetic field on the Z axis
};

struct StructBMP180{
    uint32_t timestamp;  // Timestamp
    float temp;          // Temperature
    float pressure;      // Pressure
    float altitude;      // Altitude
};

struct StructBNO055{
    uint32_t timestamp; // Timestamp
    // Acceleration
    float BNO_ax;       // BNO055 acceleration on the X axis
    float BNO_ay;       // BNO055 acceleration on the Y axis
    float BNO_az;       // BNO055 acceleration on the Z axis
    // Angular velocity
    float BNO_gx;       // BNO055 angular velocity on the X axis
    float BNO_gy;       // BNO055 angular velocity on the Y axis
    float BNO_gz;       // BNO055 angular velocity on the Z axis
    // Magnetic field
    float BNO_mx;       // BNO055 magnetic field on the X axis
    float BNO_my;       // BNO055 magnetic field on the Y axis
    float BNO_mz;       // BNO055 magnetic field on the Z axis
    // Quaternions
    float BNO_qw;       // BNO055 quaternion W component
    float BNO_qx;       // BNO055 quaternion X component
    float BNO_qy;       // BNO055 quaternion Y component
    float BNO_qz;       // BNO055 quaternion Z component
};

struct StructBME280 {
    uint32_t timestamp;  // Timestamp
    float temp;          // Temperature
    float humidity;      // Humidity
    float pressure;      // Pressure
    float altitude;      // Altitude
};

struct StructUblox {
    uint32_t timestamp;   // Timestamp

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


// Calibration data structure
struct CalibrationDataMPU {
    // MPU6050 calibration data
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
};

struct CalibrationDataQMC{
    // Hard iron offsets
    float qmcMagOffsetX;
    float qmcMagOffsetY;
    float qmcMagOffsetZ;

    // Soft iron
    float qmcMagScaleX;
    float qmcMagScaleY;
    float qmcMagScaleZ;
};

struct CalibrationDataBMP{
    float bmpPresRef;
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

extern StructMPU6050 mpuData;
extern StructQMC5883L qmcData;
extern StructBMP180 bmpData;
extern StructBNO055 bnoData;
extern StructBME280 bmeData;
extern StructUblox ubloxData;

