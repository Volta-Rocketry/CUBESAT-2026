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
#define LAUNCH_ACCEL_THRESHOLD_MS2 19.6133f ///< Acceleration threshold to detect launch
#define FAST_SAMPLE_INTERVAL_MS 10          ///< Fast sampling interval in milliseconds
#define SLOW_SAMPLE_INTERVAL_MS 1000        ///< Slow sampling interval in milliseconds

// ======================
// FLASH MEMORY COMMANDS
// ======================

/** @name FLASH COMMANDS AND DEFINITIONS */
#define FLASH_PAGE_SIZE 256UL        ///< Flash memory page size in bytes
#define FLASH_TOTAL_BYTES 4194304UL ///< Total Flash memory capacity

#define CMD_WRITE_ENABLE 0x06        ///< SPI Command: Write enable
#define CMD_CHIP_ERASE 0xC7          ///< SPI Command: Chip erase
#define CMD_PAGE_PROGRAM 0x02        ///< SPI Command: Page program
#define CMD_READ_DATA 0x03           ///< SPI Command: Read data
#define CMD_READ_STATUS 0x05         ///< SPI Command: Read status register
#define CMD_JEDEC_ID 0x9F            ///< SPI Command: Read manufacturer identification

// ============================================
// COMMUNICATION CONSTANTS, PINS AND FRECUENCY
// ============================================

#define COMM_SYNC_1 0xAA             ///< Communication synchronization Signal
#define COMM_SYNC_2 0x55             ///< Communication synchronization Signal
#define ID_CTR_TP 0x10               ///< ID of the control package
#define ID_CAM_TP 0x11               ///< ID of the camera package
#define ID_FLIGHT_SP 0X20            ///< ID of the flight package
#define ID_INIT_CMD 0XF0
#define CTR_TP_PAYLOAD_LEN 53        ///< Length of the control package payload data
#define CAM_TP_PAYLOAD_LEN 28        ///< Length of the camera package payload data
#define FLIGHT_SP_PAYLOAD_LEN 9      ///< Length of the flight package payload data
#define INIT_SP_PAYLOAD_LEN 1
#define CTR_TP_FRAME_SIZE (2 + 1 + 1 + CTR_TP_PAYLOAD_LEN + 2)       ///< Length of the control package frame
#define CAM_TP_FRAME_SIZE (2 + 1 + 1 + CAM_TP_PAYLOAD_LEN + 2)       ///< Length of the camera package frame
#define FLIGHT_SP_FRAME_SIZE (2 + 1 + 1 + FLIGHT_SP_PAYLOAD_LEN + 2) ///< Length of the flight package frame
#define INIT_SP_FRAME_SIZE (2 + 1 + 1 + INIT_SP_PAYLOAD_LEN + 2)
#define CTR_TP_INTERVAL_MS 10        ///< Time interval to send the control obc package
#define CAM_TP_INTERVAL_MS 10        ///< Time interval for camera package sending
#define FLIGHT_SP_INTERVAL_MS 1000   ///< Time interval for flight package sending
#define TEST_SP_INTERVAL_MS 1000     ///< Time interval for test package sending
#define CTR_TX 33                    ///< Reception pin (TX) to the Control OBC
#define CTR_RX 34                    ///< Reception pin (RX) to the Control OBC
#define CAM_TX 32                    ///< Reception pin (TX) to the Camera
#define CAM_RX 35                    ///< Reception pin (RX) to the Camera

#define BLINK 500

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
    float BNO_global_ax;
    float BNO_global_ay;
    float BNO_global_az;
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

// Valid
struct StructInitSensor {
    bool initBNO;
    bool initMPU;
    bool initQMC;
    bool initBMP;
    bool initBME;
    bool initGPS;
    bool initFlash;
};
struct StructCalibSensor {
    bool calibBNO;
    bool calibMPU;
    bool calibQMC;
    bool calibBMP;
    bool calibBME;
    bool calibGPS;
};
struct StructInitCom{
    bool comControl;
    bool comCamera;
};

// ==========================
// FLIGHTSTATE TYPE
// ==========================

/**
 * @enum FlightState
 * @brief List the flight states.
 */
typedef enum {
    // Test states 
    STATE_IDLE,
    STATE_INIT,
    // Flight states
    STATE_PAD,
    STATE_ASCENT,
    STATE_EYECTION,
    STATE_CONTROL,
    STATE_CUTOFF,
    STATE_RECOVERY,
    STATE_DOWNLOAD
} FlightState;

// ==========================
// FLIGHT DATA STRUCTURES
// ==========================

/**
 * @struct SlowFlightPacket
 * @brief Data package thats contains data from high frequency sensors.
 */
#pragma pack(push, 1)
typedef struct {
    uint8_t packet_id;
    uint32_t timestamp_ms;
    StructMPU6050 mpu;
    StructBNO055 bno;
    MadgwickState madgwick;
    uint16_t checksum;
} FastFlightPacket;
#pragma pack(pop)

/**
 * @struct SlowFlightPacket
 * @brief Data package thats contains data from low frequency sensors.
 */
#pragma pack(push, 1)
typedef struct {
    uint8_t packet_id;    
    uint32_t timestamp_ms;  
    StructBME280 bme;
    StructBMP180 bmpData;        
    StructUblox gps;           
    uint16_t checksum;      
} SlowFlightPacket;
#pragma pack(pop)

// ==========================
// COMMUNICATION DATA STRUCTURES
// ==========================

/**
 * @struct CommsCtrData
 * @brief Data package for sending to Control OBC.
 */
struct CommsCtrData{
    uint32_t timestamp; // Timestamp
    float altitude; // BME280 altitude
    float vertical_velocity; // Vertical velocity
    float ax, ay, az; // BNO055 acceleration on the X, Y and Z axis
    float gx, gy, gz; // BNO055 angular velocity on the X, Y and Z axis
    float qw, qx, qy, qz; // BNO055 quaternion W, X, Y and Z components
    FlightState flight_state; // Flight State
};

/**
 * @struct CommsCamData
 * @brief Data package for sending to Camera.
 */
struct CommsCamData{
    uint32_t timestamp; // Timestamp
    float ax, ay, az; // BNO055 acceleration on the X, Y and Z axis
    float gx, gy, gz; // BNO055 angular velocity on the X, Y and Z axis
};

struct CommsInitData{
    uint8_t id_to_init;
};

// ==========================
// FILTERS DATA STRUCTURES
// ==========================

struct MadgwickState{
    float q0;    ///< Componente escalar del cuaternión (parte real)
    float q1;    ///< Componente vectorial i (eje X)                           
    float q2;    ///< Componente vectorial j (eje Y)                             
    float q3;    ///< Componente vectorial k (eje Z)                             
    float beta;  ///< Ganancia del gradiente descendente [rad/s]                  
};

struct EulerAngles{
    float roll;    ///< rotación alrededor de x
    float pitch;   ///< rotación alrededor de y
    float yaw;     ///< rotación alrededor de z
};

// ==========================
// EXTERNAL GLOBAL VARIABLES
// ==========================

/** @name Sensor Data Instances
 * Global variables defined in the main source file for access throughout the program.
 */
extern StructMPU6050 mpuData;           ///< Global instance for MPU6050 data
extern StructQMC5883L qmcData;          ///< Global instance for QMC5883L data
extern StructBMP180 bmpData;            ///< Global instance for BMP180 data
extern StructBNO055 bnoData;            ///< Global instance for BNO055 data
extern StructBME280 bmeData;            ///< Global instance for BME280 data
extern StructUblox ubloxData;           ///< Global instance for Ublox GPS data
extern CommsCtrData ctrData;            ///< Global instance for data to be sent to Control
extern CommsCamData camData;            ///< Global instance for data to be sent to Camera
extern CommsInitData initData;
extern StructInitSensor initSensor;
extern StructCalibSensor calibSensor;
extern StructInitCom initCom;
extern FastFlightPacket fastP;
extern SlowFlightPacket slowP;
extern MadgwickState madgwickState;
extern EulerAngles eulerAngles;