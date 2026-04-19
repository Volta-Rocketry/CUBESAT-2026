#include "sensors_manager.h"

#include "constants.h"
#include "error_warning.h"
#include <Arduino.h>
#include "MPU9250.h"
#include <Adafruit_BNO055.h>
#include <Adafruit_BME280.h>
#include <Adafruit_PCF8574.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h> 
#include <utility/imumaths.h>
#include <TinyGPSplus.h> 
#include <HardwareSerial.h>
#include <Preferences.h>

MPU9250 mpu(SPI,MPU_CS);


Adafruit_BNO055 bno;
Adafruit_BME280 bme(BME_CS);
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);
Adafruit_PCF8574 pcf;

StructMPU9250 mpuData;
StructBNO055 bnoData;
StructBME280 bmeData;
StructUblox ubloxData;

CalibrationDataMPU mpuCalib;
CalibrationDataBNO bnoCalib;
CalibrationDataBME bmeCalib;

Preferences preferences;

int numCalib = 0;

void InitExtencionBoard() {
    Wire.begin(21, 22);
    Wire.setTimeOut(100);
    delay(100); 

    if (!pcf.begin(0x20, &Wire)) {
        CriticalErrorSensor("PCF8574 initialization failed");
    } else {
        Serial.println("PCF8574 I/O expander initialized successfully.");
        pcf.pinMode(ACTUATOR1_PIN, OUTPUT);
        pcf.pinMode(ACTUATOR2_PIN, OUTPUT);
        pcf.pinMode(SD_CS, OUTPUT);
        pcf.pinMode(BUZZER_PIN, OUTPUT);
        pcf.pinMode(LED_RED_PIN, OUTPUT);
        pcf.pinMode(LED_GREEN_PIN, OUTPUT);
        pcf.pinMode(LED_BLUE_PIN, OUTPUT);

        // Aseguramos que los PINES inicien apagados por seguridad
        pcf.digitalWrite(ACTUATOR1_PIN, LOW);
        pcf.digitalWrite(ACTUATOR2_PIN, LOW);
        pcf.digitalWrite(BUZZER_PIN, HIGH);
        pcf.digitalWrite(LED_RED_PIN, LOW);
        pcf.digitalWrite(LED_GREEN_PIN, LOW);
        pcf.digitalWrite(LED_BLUE_PIN, LOW);

        Serial.println("PCF8574 pins configured successfully.");

        digitalWrite(BUZZER_PIN, LOW);
        delay(1000);
        digitalWrite(BUZZER_PIN, HIGH);

    }
}

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
    gpsSerial.begin(GPS_BAUD,SERIAL_8N1,UBLOX_RX,UBLOX_TX);

    unsigned long startTime = millis();
    bool GPSInitialized = false;

    while (millis() - startTime < 3000) {
        if (gpsSerial.available() > 0) {
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

void InitActuators() {
    // Code to initialize actuators
    pinMode(ACTUATOR1_PIN, OUTPUT);
    pinMode(ACTUATOR2_PIN, OUTPUT);
    if (digitalRead(ACTUATOR1_PIN) == HIGH || digitalRead(ACTUATOR2_PIN) == HIGH) {
        CriticalErrorSensor("Actuators initialization failed");
    }
    Serial.println("Actuators initialized successfully.");
}

// Sensor calibration functions
void CalibrateSensors() {
    // Variables temporales para el proceso de calibración
    float gSumX = 0, gSumY = 0, gSumZ = 0;
    float aSumX = 0, aSumY = 0, aSumZ = 0;
    float tsum = 0;
    float pSum = 0;
    bool GPSConected = false;
    int numReadings = 0;
    const int MAX_MU = 1000;
    bool giroCalibrado = false;
    uint32_t previousCalibMilis = 0;
    // Data collection loop for calibration
/*
    while (numReadings < MAX_MU) {
        uint32_t calibMilis = millis();
        if (calibMilis - previousCalibMilis >= 150) {
            previousCalibMilis = calibMilis;
            
             if (mpu.readSensor() == 0) {
                gSumX += mpu.getGyroX_rads();
                gSumY += mpu.getGyroY_rads();
                gSumZ += mpu.getGyroZ_rads();
                aSumX += mpu.getAccelX_mss();
                aSumY += mpu.getAccelY_mss();
                aSumZ += mpu.getAccelZ_mss();

                tsum += mpu.getTemperature_C();
                }
                
            pSum += bme.readPressure();

            numReadings++;
        }
    }
*/
    // MPU9250 calibration
    /*
    mpuCalib.mpuGyroBiasX = gSumX / float(numReadings);
    mpuCalib.mpuGyroBiasY = gSumY / float(numReadings);
    mpuCalib.mpuGyroBiasZ = gSumZ / float(numReadings);
    mpuCalib.mpuAccBiasX = aSumX / float(numReadings);
    mpuCalib.mpuAccBiasY = aSumY / float(numReadings);
    mpuCalib.mpuAccBiasZ = aSumZ / float(numReadings) - 9.80665;

    // Calibración de temperatura
    mpuCalib.tempRef = tsum / float(numReadings);
    mpuCalib.gyroTCO = 0.000872; // 0.5 deg/s * (pi / 180)= 0.00872665 rad/s
    mpuCalib.accTCO = 0.0147;  // 1.5 mg = 0.0015 G * 9.80665 = 0.0147 m/s^2
*/
/*
    // Get calibration from BNO055
    bool calibrated = false;
    bno.getCalibration(&bnoCalib.bnoSystemStatus, &bnoCalib.bnoGyroStatus, &bnoCalib.bnoAccStatus, &bnoCalib.bnoMagStatus);
    Serial.printf("BNO055 calibration status - System: %d, Gyro: %d, Accel: %d, Mag: %d\n", bnoCalib.bnoSystemStatus, bnoCalib.bnoGyroStatus, bnoCalib.bnoAccStatus, bnoCalib.bnoMagStatus);
    if (bno.isFullyCalibrated() && !calibrated) {
        Serial.println("BNO055 sensor fully calibrated.");
        adafruit_bno055_offsets_t newOffsets;
        preferences.begin("bno_data", false);
        bno.getSensorOffsets(newOffsets);
        preferences.putBytes("bno_offsets", &newOffsets, sizeof(newOffsets));
        preferences.end();
        Serial.println("BNO055 calibration data saved to preferences.");
        calibrated = true;
    }
    if (!bno.isFullyCalibrated()) {
        preferences.begin("bno_data", true);
        size_t dataLength = preferences.getBytesLength("bno_offsets");
        if (dataLength > 0 ){
            adafruit_bno055_offsets_t savedOffsets;
            preferences.getBytes("bno_offsets", &savedOffsets, sizeof(savedOffsets));
            bno.setSensorOffsets(savedOffsets);
            Serial.println("BNO055 calibration data loaded.");
            preferences.end();
        } 
    else {
        Serial.println("No BNO055 calibration data found.");
        }
    }
*/
    while (!bno.isFullyCalibrated()) {
        bno.getCalibration(&bnoCalib.bnoSystemStatus, &bnoCalib.bnoGyroStatus, &bnoCalib.bnoAccStatus, &bnoCalib.bnoMagStatus);
        Serial.printf("BNO055 calibration status - System: %d, Gyro: %d, Accel: %d, Mag: %d\n", bnoCalib.bnoSystemStatus, bnoCalib.bnoGyroStatus, bnoCalib.bnoAccStatus, bnoCalib.bnoMagStatus);
        Serial.println("Calibrating BNO055... Please follow the calibration procedure.");
        if (bnoCalib.bnoGyroStatus < 3){
            Serial.println("Calibrate Gyroscope: Keep the sensor stationary on a flat surface.");
        }
        else if (bnoCalib.bnoGyroStatus == 3){
            Serial.println("Gyroscope calibrated.");
        }
        else if (bnoCalib.bnoMagStatus < 3){
            Serial.println("Calibrate Magnetometer: Move the sensor in a figure-eight pattern.");
        }
        else if (bnoCalib.bnoMagStatus == 3){
            Serial.println("Magnetometer calibrated.");
        } 
        if (bnoCalib.bnoAccStatus < 3){
            Serial.println("Calibrate Accelerometer: Move the sensor in different positions.");
        }
        else if (bnoCalib.bnoAccStatus == 3){
            Serial.println("Accelerometer calibrated.");
        }
        delay(1000);
    }
    if (bno.isFullyCalibrated()){
        Serial.println("BNO055 sensor fully calibrated.");
    }
    /*
    // BME280 calibration
    bmeCalib.bmePresRef = pSum / float(numReadings);
    Serial.println("BME280 calibration completed successfully.");
    numCalib = 1;

    // GPS connection check
    gpsSerial.begin(GPS_BAUD,SERIAL_8N1,UBLOX_RX,UBLOX_TX);
    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
    }
    if (!GPSConected && gps.location.isValid() && gps.satellites.value() > 3) {
        GPSConected = true;
        numCalib = 2;
        Serial.println("GPS connected successfully.");
    } else if (!GPSConected) {
        Serial.println("GPS connection failed during calibration.");
    }
        */
}

void CalibratMagnetometer() {
    float magMin[3] = {32767, 32767, 32767};
    float magMax[3] = {-32768, -32768, -32768};

    float previousCalibMagSec = 0;
    bool calibrated = false;
    while (!calibrated) {
        float calibMagSec = millis() / 1000.0;
        if (calibMagSec - previousCalibMagSec >= 0.1) {
            previousCalibMagSec = calibMagSec;
            if (mpu.readSensor() == 0) {
                float mx = mpu.getMagX_uT();
                float my = mpu.getMagY_uT();
                float mz = mpu.getMagZ_uT();

                if (mx < magMin[0]) magMin[0] = mx;
                if (my < magMin[1]) magMin[1] = my;
                if (mz < magMin[2]) magMin[2] = mz;

                if (mx > magMax[0]) magMax[0] = mx;
                if (my > magMax[1]) magMax[1] = my;
                if (mz > magMax[2]) magMax[2] = mz;
            }
            if (previousCalibMagSec == 30){
                calibrated = true;
            }
        }
    }
    mpuCalib.mpuMagOffsetX = (magMax[0] + magMin[0]) / 2.0;
    mpuCalib.mpuMagOffsetY = (magMax[1] + magMin[1]) / 2.0;
    mpuCalib.mpuMagOffsetZ = (magMax[2] + magMin[2]) / 2.0;

    float radioX = (magMax[0] - magMin[0]) / 2.0;
    float radioY = (magMax[1] - magMin[1]) / 2.0;
    float radioZ = (magMax[2] - magMin[2]) / 2.0;

    float radioPromedio = (radioX + radioY + radioZ) / 3.0;

    mpuCalib.mpuMagScaleX = radioPromedio / radioX;
    mpuCalib.mpuMagScaleY = radioPromedio / radioY;
    mpuCalib.mpuMagScaleZ = radioPromedio / radioZ;
}

// Sensor reading functions
void ReadMPU9250() {
    // Code to read data from MPU9250 sensor
    // temperatura correcion
    // Correcion del error es:
    // Dato corregido = Dato medido - (Bias + TCO * (Temperatura actual - Temperatura de referencia))
    // TCO (Temperature Coefficient of Offset) sale del datasheet
    float deltaT = (mpu.getTemperature_C() - mpuCalib.tempRef);

    mpuData.timestamp = millis();

    mpuData.MPU_ax = mpu.getAccelX_mss(); // - (mpuCalib.mpuAccBiasX + (mpuCalib.accTCO * deltaT));
    mpuData.MPU_ay = mpu.getAccelY_mss(); // - (mpuCalib.mpuAccBiasY + (mpuCalib.accTCO * deltaT));
    mpuData.MPU_az = mpu.getAccelZ_mss(); // - (mpuCalib.mpuAccBiasZ + (mpuCalib.accTCO * deltaT));

    mpuData.MPU_gx = mpu.getGyroX_rads(); // - (mpuCalib.mpuGyroBiasX + (mpuCalib.gyroTCO * deltaT));
    mpuData.MPU_gy = mpu.getGyroY_rads(); // - (mpuCalib.mpuGyroBiasY + (mpuCalib.gyroTCO * deltaT));
    mpuData.MPU_gz = mpu.getGyroZ_rads(); // - (mpuCalib.mpuGyroBiasZ + (mpuCalib.gyroTCO * deltaT));

    mpuData.MPU_mx = mpu.getMagX_uT();
    mpuData.MPU_my = mpu.getMagY_uT();
    mpuData.MPU_mz = mpu.getMagZ_uT();

}
void ReadBNO055() {
    // Code to read data from BNO055 sensor
    bnoData.timestamp = millis();
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
     imu::Quaternion quat = bno.getQuat();
    bnoData.BNO_qw = quat.w();
    bnoData.BNO_qx = quat.x();
    bnoData.BNO_qy = quat.y();
    bnoData.BNO_qz = quat.z();
}
void ReadBME280() {
    // Code to read data from BME sensor
    float pressurePad = bmeCalib.bmePresRef / 100.0; 
    bmeData.timestamp = millis();
    bmeData.temp = bme.readTemperature();
    bmeData.humidity = bme.readHumidity();
    bmeData.pressure = bme.readPressure();
    bmeData.altitude = bme.readAltitude(pressurePad); // Using standard sea level pressure as reference
   //bmeData.altitude = bme.readAltitude(bmeCalib.bmePresRef / 100.0);
}
void ReadUblox() {
    // Code to read data from Ublox sensor
    ubloxData.timestamp = millis();
    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
    }
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


// Actuator control functions
void OpenActuatorsVoltage() {
    // Code to send voltage to actuators
    int blinkCount = 0;
    float previousSecs = 0;
    bool ledState = LOW; 
    float secs_actuators = millis() / 1000.0;
    digitalWrite(ACTUATOR1_PIN, HIGH);
    digitalWrite(ACTUATOR2_PIN, HIGH);
    while (blinkCount < 6) {
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
    digitalWrite(ACTUATOR1_PIN, LOW);
    digitalWrite(ACTUATOR2_PIN, LOW);
}