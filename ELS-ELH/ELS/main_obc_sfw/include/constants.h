#pragma once

#include <stdint.h>
/**
 * @brief Pins definitions and data structure definitions.
 *
 * This file contains all the pin configurations for the sensors (GY87, BNO055, BME280, UBLOX),
 * memory configuration (Flash, SD), actuators, and the necessary structures to store telemetry and calibration data.
 */

/** @name Baud Rates */ 
#define BAUD_RATE 115200  ///< Baude rate for the main serial monitor
#define GPS_BAUD 9600  ///< Baud rate for the Ublox GPS module

//============================
// PIN DEFINITIONS (HARDWARE)
//============================

/** @name GY87 MODULE (MPU6050, QMC5883L, BMP180) */
#define GY_SCL 22  ///< I2C Clock pin for the GY87
#define GY_SDA 21  ///< I2C Data pin for the GY87

/** @name BNO055 SENSOR */
#define BNO_SDA 21  ///< I2C Clock pin for the BNO055
#define BNO_SCL 22  ///< I2C Data pin for the BNO055

/** @name BME280 SENSOR */
#define BME_SCK 18   ///< SPI Clock pin for the BME280
#define BME_MOSI 23  ///< SPI Mosi pin for the BME280
#define BME_MISO 19  ///< SPI Miso pin for the BME280
#define BME_CS 26    ///< SPI Chip select pin for the BME280

/** @name UBLOX GPS Module */
#define UBLOX_TX 25  ///< Transmission pin (TX) to the GPS
#define UBLOX_RX 36  ///< Reception pin (RX) to the GPS

/** @name EXTERNAL FLASH MEMORY */
#define FLASH_SCK 18   ///< SPI Clock pin for the FLASH MEMORY
#define FLASH_MOSI 23  ///< SPI Mosi pin for the FLASH MEMORY
#define FLASH_MISO 19  ///< SPI Miso pin for the FLASH MEMORY
#define FLASH_CS 5     ///< SPI Chip select pin for the FLASH MEMORY

/** @name SD CARD MODULE */
#define SD_SCK 18   ///< SPI Clock pin for the SD CARD
#define SD_MOSI 22  ///< SPI Mosi pin for the SD CARD
#define SD_MISO 19  ///< SPI Miso pin for the SD CARD
#define SD_CS  2    ///< SPI Chip select pin for the SD CARD

/** @name ACTUATORS */
#define ACTUATOR1_PIN 16 ///< Control pin for Actuator 1
#define ACTUATOR2_PIN 4  ///< Control pin for Actuator 2

/** @name RGB LED PINS */
#define LED_RED_PIN 13   ///< Physical pin for the RGB LED red color
#define LED_GREEN_PIN 14 ///< Physical pin for the RGB LED green color
#define LED_BLUE_PIN 27  ///< Physical pin for the RGB LED blue color

/** @name BUZZER PIN*/ 
#define BUZZER_PIN 17  ///< Physical pin for the BUZZER

// ==================
// PWM CONFIGURATION
// ==================

/** @name PWM CHANELS AND PARAMETERS */
#define RED_CHANNEL 0      ///< Internal PWM channel for the Red LED
#define GREEN_CHANNEL 1    ///< Internal PWM channel for the Green LED
#define BLUE_CHANNEL 2     ///< Internal PWM channel for the Blue LED
#define BUZZER_CHANNEL 3   ///< Internal PWM channel for the Buzzer

#define PWM_FREQ 5000      ///< Base PWM signal frequency in Hz
#define PWM_RESOLUTION 8   ///< PWM resolution in bits

// ===========================
// FLIGHT COMPUTER PARAMETERS
// ===========================

/** @name FLIGHT LOGIC AND SAMPLING */
#define LAUNCH_ACCEL_THRESHOLD_MS2 29.43f  ///< Acceleration threshold to detect launch
#define FAST_SAMPLE_INTERVAL_MS 10         ///< Fast sampling interval in milliseconds
#define SLOW_SAMPLE_INTERVAL_MS 1000       ///< Slow sampling interval in milliseconds

// ======================
// FLASH MEMORY COMMANDS
// ======================

/** @name FLASH COMMANDS AND DEFINITIONS */
#define FLASH_PAGE_SIZE 256UL          ///< Flash memory page size in bytes
#define FLASH_TOTAL_BYTES 16777216UL   ///< Total Flash memory capacity

#define CMD_WRITE_ENABLE 0x06          ///< SPI Command: Write enable
#define CMD_CHIP_ERASE 0xC7            ///< SPI Command: Chip erase
#define CMD_PAGE_PROGRAM 0x02          ///< SPI Command: Page program
#define CMD_READ_DATA 0x03             ///< SPI Command: Read data
#define CMD_READ_STATUS 0x05           ///< SPI Command: Read status register
#define CMD_JEDEC_ID 0x9F              ///< SPI Command: Read manufacturer identification

// COMMUNICATION
#define COMM_SYNC_1 0xAA
#define COMM_SYNC_2 0x55
#define ID_CTR_TP 0x10
#define ID_CAM_TP 0x11
#define ID_FLIGHT_SP 0X20
#define CTR_TP_PAYLOAD_LEN 53
#define CAM_TP_PAYLOAD_LEN 28
#define FLIGHT_SP_PAYLOAD_LEN 9
#define CTR_TP_FRAME_SIZE (2 + 1 + 1 + CTR_TP_PAYLOAD_LEN + 2)
#define CAM_TP_FRAME_SIZE (2 + 1 + 1 + CAM_TP_PAYLOAD_LEN + 2)
#define FLIGHT_SP_FRAME_SIZE (2 + 1 + 1 + FLIGHT_SP_PAYLOAD_LEN + 2)
#define CTR_TP_INTERVAL_MS 10
#define CAM_TP_INTERVAL_MS 10
#define FLIGHT_SP_INTERVAL_MS 1000
#define TEST_SP_INTERVAL_MS 1000
#define CTR_TX 43
#define CTR_RX 44
#define CAM_TX 15
#define CAM_RX 16

// =======================
// SENSOR DATA STRUCTURES
// =======================

/**
 * @struct StructMPU6050
 * @brief Stores 6-axis inertial data and temperature from the MPU6050.
 */
struct StructMPU6050{
    uint32_t timestamp;  ///< Reading timestamp in ms
    float MPU_ax;        ///< Acceleration on the X axis
    float MPU_ay;        ///< Acceleration on the Y axis
    float MPU_az;        ///< Acceleration on the Z axis
    float MPU_gx;        ///< Angular velocity on the X axis
    float MPU_gy;        ///< Angular velocity on the Y axis
    float MPU_gz;        ///< Angular velocity on the Z axis
    float MPU_temp;      ///< Internal chip temperature
};

/**
 * @struct StructQMC5883L
 * @brief Stores data from the QMC5883L magnetometer.
 */
struct StructQMC5883L{
    uint32_t timestamp; ///< Reading timestamp in ms
    float QMC_mx;       ///< Magnetic field on the X axis
    float QMC_my;       ///< Magnetic field on the Y axis
    float QMC_mz;       ///< Magnetic field on the Z axis
};

/**
 * @struct StructBMP180
 * @brief Stores barometric data from the BMP180 sensor.
 */
struct StructBMP180{
    uint32_t timestamp;  ///< Reading timestamp in ms
    float temp;          ///< Temperature in Celsius degrees
    float pressure;      ///< Atmospheric pressure in Pa
    float altitude;      ///< Calculated altitude in meters
};

/**
 * @struct StructBNO055
 * @brief Stores 6-axis inertial data, magnetometer, and quaternions from the BNO055.
 */
struct StructBNO055{
    uint32_t timestamp; ///< Reading timestamp in ms
    float BNO_ax;       ///< Acceleration on the X axis
    float BNO_ay;       ///< Acceleration on the Y axis
    float BNO_az;       ///< Acceleration on the Z axis
    float BNO_gx;       ///< Angular velocity on the X axis
    float BNO_gy;       ///< Angular velocity on the Y axis
    float BNO_gz;       ///< Angular velocity on the Z axis
    float BNO_mx;       ///< Magnetic field on the X axis
    float BNO_my;       ///< Magnetic field on the Y axis
    float BNO_mz;       ///< Magnetic field on the Z axis
    float BNO_qw;       ///< Quaternion orientation W component
    float BNO_qx;       ///< Quaternion orientation X component
    float BNO_qy;       ///< Quaternion orientation Y component
    float BNO_qz;       ///< Quaternion orientation Z component
};

/**
 * @struct StructBME280
 * @brief Stores barometric data from the BME280 sensor.
 */
struct StructBME280 {
    uint32_t timestamp;  ///< Reading timestamp in ms
    float temp;          ///< Temperature in Celsius degrees
    float humidity;      ///< Relative humidity in percentage (%)
    float pressure;      ///< Atmospheric pressure in Pa
    float altitude;      ///< Calculated altitude in meters
};

/**
 * @struct StructUblox
 * @brief Stores positioning and time data obtained from the Ublox GPS.
 */
struct StructUblox {
    uint32_t timestamp;   ///< Local system timestamp in ms
    
    // GPS Time
    uint8_t hour;         ///< UTC Hour
    uint8_t minute;       ///< UTC Minute
    uint8_t second;       ///< UTC Second
    
    // GPS Date
    uint16_t year;        ///< Year
    uint8_t month;        ///< Month
    uint8_t day;          ///< Day
    
    double latitude;      ///< Latitude in decimal degrees
    double longitude;     ///< Longitude in decimal degrees
    float altitude;       ///< Altitude above mean sea level in meters
    float speed;          ///< Ground speed
    float course;         ///< Course over ground
    uint8_t satellites;   ///< Number of satellites
    float hdop;           ///< Horizontal Dilution of Precision
    bool valid;           ///< GPS data validity flag
};

// ============================
// CALIBRATION DATA STRUCTURES
// ============================

/**
 * @struct CalibrationDataMPU
 * @brief Calibration offsets and coefficients for the MPU6050.
 */
struct CalibrationDataMPU {
    float mpuGyroBiasX;   ///< Gyroscope offset on X
    float mpuGyroBiasY;   ///< Gyroscope offset on Y
    float mpuGyroBiasZ;   ///< Gyroscope offset on Z
    float mpuAccBiasX;    ///< Accelerometer offset on X
    float mpuAccBiasY;    ///< Accelerometer offset on Y
    float mpuAccBiasZ;    ///< Accelerometer offset on Z
    float tempRef;        ///< Reference temperature for calibration
    float gyroTCO;        ///< Temperature coefficient for the gyroscope
    float accTCO;         ///< Temperature coefficient for the accelerometer
};

/**
 * @struct CalibrationDataQMC
 * @brief Hard-Iron and Soft-Iron calibration parameters for the QMC5883L.
 */
struct CalibrationDataQMC{
    float qmcMagOffsetX;  ///< Hard-Iron offset on X
    float qmcMagOffsetY;  ///< Hard-Iron offset on Y
    float qmcMagOffsetZ;  ///< Hard-Iron offset on Z
    float qmcMagScaleX;   ///< Soft-Iron scale factor on X
    float qmcMagScaleY;   ///< Soft-Iron scale factor on Y
    float qmcMagScaleZ;   ///< Soft-Iron scale factor on Z
};

/**
 * @struct CalibrationDataBMP
 * @brief Calibration data for the BMP180.
 */
struct CalibrationDataBMP{
    float bmpPresRef;     ///< Reference pressure for altitude calculation
};

/**
 * @struct CalibrationDataBNO
 * @brief Calibration status of the BNO055 internal system.
 */
struct CalibrationDataBNO {
    uint8_t bnoSystemStatus; ///< General system calibration level (0-3)
    uint8_t bnoGyroStatus;   ///< Gyroscope calibration level (0-3)
    uint8_t bnoAccStatus;    ///< Accelerometer calibration level (0-3)
    uint8_t bnoMagStatus;    ///< Magnetometer calibration level (0-3)
};

/**
 * @struct CalibrationDataBME
 * @brief Calibration data for the BME280.
 */
struct CalibrationDataBME {
    float bmePresRef;        ///< Reference pressure for altitude calculation
};

// ==========================
// EXTERNAL GLOBAL VARIABLES
// ==========================

/** @name Sensor Data Instances
 * Global variables defined in the main source file for access throughout the program.
 */
extern StructMPU6050 mpuData;    ///< Global instance for MPU6050 data
extern StructQMC5883L qmcData;   ///< Global instance for QMC5883L data
extern StructBMP180 bmpData;     ///< Global instance for BMP180 data
extern StructBNO055 bnoData;     ///< Global instance for BNO055 data
extern StructBME280 bmeData;     ///< Global instance for BME280 data
extern StructUblox ubloxData;    ///< Global instance for Ublox GPS data

