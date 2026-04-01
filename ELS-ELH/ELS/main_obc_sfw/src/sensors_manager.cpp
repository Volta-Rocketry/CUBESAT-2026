#include "sensors_manager.h"

#include "constants.h"
#include "error_warning.h"
#include <Arduino.h>
#include "MPU9250.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h> 
#include <Adafruit_BNO055.h>
#include <Adafruit_BME280.h>
#include <utility/imumaths.h>
//#include <TinyGPSplus.h> 

MPU9250 mpu(SPI, 47);               // SPI ADREESS 0x68
// MPU9250 mpu(SPI, MPU_CS);

Adafruit_BNO055 bno;      // I2C ADDRESS 0x28


Adafruit_BME280 bme(14);     // SPI ADRESS 0x76
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

StructMPU9250 mpuData;
StructBNO055 bnoData;
StructBME280 bmeData;


void InitMPU9250() {
    // Code to initialize MPU9250 sensor
    if (mpu.begin() < 0) {
        criticalError("MPU9250 initialization failed");
    } else {
        Serial.println("MPU9250 sensor initialized successfully.");
    };
}
void InitBNO055() {
    // Code to initialize BNO055 sensor
    if (!bno.begin()) {
        criticalError("BNO055 initialization failed");
    } else {
        Serial.println("BNO055 sensor initialized successfully.");
    };
}
void InitBME280() {
    // Code to initialize BME280 sensor
    if (!bme.begin()) {
        criticalError("BME280 initialization failed");
    } else {
        Serial.println("BME280 sensor initialized successfully.");
    }
}
void InitUblox() {
    // Code to initialize Ublox sensor
}
void InitTransducers() {
    // Code to initialize transducers
}


void calibrateSensors() {
    // Code to calibrate sensors
}


void ReadMPU9250() {
    // Code to read data from MPU9250 sensor
    mpuData.timestamp = millis() / 1000.0;
    mpuData.MPU_ax = mpu.getAccelX_mss();
    mpuData.MPU_ay = mpu.getAccelY_mss();
    mpuData.MPU_az = mpu.getAccelZ_mss();
    mpuData.MPU_gx = mpu.getGyroX_rads();
    mpuData.MPU_gy = mpu.getGyroY_rads();
    mpuData.MPU_gz = mpu.getGyroZ_rads();
    mpuData.MPU_mx = mpu.getMagX_uT();
    mpuData.MPU_my = mpu.getMagY_uT();
    mpuData.MPU_mz = mpu.getMagZ_uT();
}
void ReadBNO055() {
    // Code to read data from BNO055 sensor
    bnoData.timestamp = millis() / 1000.0;
    imu::Vector<3> lin_accel = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
    bnoData.BNO_ax = lin_accel.x();
    bnoData.BNO_ay = lin_accel.y();
    bnoData.BNO_az = lin_accel.z();
    imu::Vector<3> gyro = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
    bnoData.BNO_gx = gyro.x();
    bnoData.BNO_gy = gyro.y();
    bnoData.BNO_gz = gyro.z();
    imu::Vector<3> mag = bno.getVector(Adafruit_BNO055::VECTOR_MAGNETOMETER);
    bnoData.BNO_mx = mag.x();
    bnoData.BNO_my = mag.y();
    bnoData.BNO_mz = mag.z();
}
void ReadBME280() {
    // Code to read data from BME sensor
    bmeData.timestamp = millis() / 1000.0;
    bmeData.temp = bme.readTemperature();
    bmeData.humidity = bme.readHumidity();
    bmeData.pressure = bme.readPressure() / 100.0F;
    // Altitude can be calculated using the pressure reading and a reference sea level pressure
}
void ReadUblox() {
    // Code to read data from Ublox sensor
}
void ReadTransducers() {
    // Code to read data from transducers
}