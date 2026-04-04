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
#include <TinyGPSplus.h> 
#include <SoftwareSerial.h>

MPU9250 mpu(SPI,MPU_CS);               // SPI ADREESS 0x68
// MPU9250 mpu(SPI, MPU_CS);

Adafruit_BNO055 bno;      // I2C ADDRESS 0x28


Adafruit_BME280 bme(BME_CS);     // SPI ADRESS 0x76
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

TinyGPSPlus gps;

StructMPU9250 mpuData;
StructBNO055 bnoData;
StructBME280 bmeData;
StructUblox ubloxData;
StructTransducer transducerData;

// Sensor initialization functions
void InitMPU9250() {
    // Code to initialize MPU9250 sensor
    if (mpu.begin() < 0) {
        CriticalErrorSensor("MPU9250 initialization failed");
    } else {
        Serial.println("MPU9250 sensor initialized successfully.");
    };
}
void InitBNO055() {
    // Code to initialize BNO055 sensor
    if (!bno.begin()) {
        CriticalErrorSensor("BNO055 initialization failed");
    } else {
        Serial.println("BNO055 sensor initialized successfully.");
    };
}
void InitBME280() {
    // Code to initialize BME280 sensor
    if (!bme.begin()) {
        CriticalErrorSensor("BME280 initialization failed");
    } else {
        Serial.println("BME280 sensor initialized successfully.");
    }
}
void InitUblox() {
    // Code to initialize Ublox sensor
    SoftwareSerial ss(UBLOX_RX, UBLOX_TX);
    ss.begin(GPS_BAUD);

    unsigned long startTime = millis();
    bool GPSInitialized = false;

    while (millis() - startTime < 3000) {
        if (ss.available() > 0) {
            GPSInitialized = true; 
            break;
        }
    }
    if (!GPSInitialized) {
        CriticalErrorSensor("Ublox initialization failed");
    } else {
        Serial.println("Ublox sensor initialized successfully.");
    }
}
void InitTransducers() {
    // Code to initialize transducers
    pinMode(TRANSDUCER_PIN, INPUT);
    if (analogRead(TRANSDUCER_PIN) <= 0 || analogRead(TRANSDUCER_PIN) >= 4095) {
        CriticalErrorSensor("Transducers Error");
    }
    Serial.println("Transducers initialized successfully.");
}

void InitActuators() {
    // Code to initialize actuators
    pinMode(ACTUATOR_PIN, OUTPUT);
    if (digitalRead(ACTUATOR_PIN) == HIGH) {
        CriticalErrorSensor("Actuators initialization failed");
    }
    Serial.println("Actuators initialized successfully.");
}

// Sensor calibration functions
void calibrateSensors() {
    // Code to calibrate sensors
}


// Sensor reading functions
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
    bmeData.pressure = bme.readPressure();
    bmeData.altitude = bme.readAltitude(BME_PRESSURE_LEVEL);
}
void ReadUblox() {
    // Code to read data from Ublox sensor
    ubloxData.timestamp = millis() / 1000.0;
    if (gps.time.isUpdated()) {
        ubloxData.hour = gps.time.hour();
        ubloxData.minute = gps.time.minute();
        ubloxData.second = gps.time.second();
    }
    if (gps.date.isUpdated()) {
        ubloxData.year = gps.date.year();
        ubloxData.month = gps.date.month();
        ubloxData.day = gps.date.day();
    }
    if (gps.location.isUpdated()) {
        ubloxData.latitude = gps.location.lat();
        ubloxData.longitude = gps.location.lng();
    }
    if (gps.altitude.isUpdated()) {
        ubloxData.altitude = gps.altitude.meters();
    }
    if (gps.speed.isUpdated()) {
        ubloxData.speed = gps.speed.mps();
    }
    if (gps.course.isUpdated()) {
        ubloxData.course = gps.course.deg();
    }
    if (gps.satellites.isUpdated()) {
        ubloxData.satellites = gps.satellites.value();
    }
    if (gps.hdop.isUpdated()) {
        ubloxData.hdop = gps.hdop.hdop();
    }
    ubloxData.valid = gps.location.isValid();
}

void ReadTransducers() {
    // Code to read data from transducers
    float transducerValue = analogRead(TRANSDUCER_PIN);
    float voltage = transducerValue * (3.3 / 4095.0);
    float pressureTransducer = voltage * 100.0; // Example conversion, adjust is needed
    transducerData.timestamp = millis() / 1000.0;
    transducerData.voltage = voltage;
    transducerData.pressureTransducer = pressureTransducer;
}

// Actuator control functions
void OpenActuatorsVoltage() {
    // Code to send voltage to actuators
    int blinkCount = 0;
    float previousSecs = 0;
    bool ledState = LOW; 
    float secs_actuators = millis() / 1000.0;
    digitalWrite(ACTUATOR_PIN, HIGH);
    if (blinkCount < 6) {
        if (secs_actuators - previousSecs >= 1.0) {
            previousSecs = secs_actuators;
            ledState = !ledState;
            digitalWrite(LED_RED_PIN, ledState);
            digitalWrite(LED_GREEN_PIN, ledState);

            blinkCount++;
        }
    }
}
void CloseActuatorsVoltage() {
    // Code to stop voltage to actuators
    digitalWrite(ACTUATOR_PIN, LOW);
}