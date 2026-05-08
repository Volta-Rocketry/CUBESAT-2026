#include "sensors_manager.h"

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "constants.h"
#include "error_warning.h"
#include "signals.h"
#include <Adafruit_BNO055.h>
#include <Adafruit_BME280.h>
#include <MPU6050_light.h>
#include <DFRobot_QMC5883.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_Sensor.h> 
#include <utility/imumaths.h>
#include <TinyGPSplus.h> 
#include <HardwareSerial.h>
#include <Preferences.h>
#include "BluetoothSerial.h"

extern BluetoothSerial SerialBT;
MPU6050 mpu(Wire);
Adafruit_BMP085 bmp;
Adafruit_BNO055 bno;
Adafruit_BME280 bme(BME_CS);
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

StructMPU6050 mpuData;
StructQMC5883L qmcData;
StructBMP180 bmpData;
StructBNO055 bnoData;
StructBME280 bmeData;
StructUblox ubloxData;

CalibrationDataMPU mpuCalib;
CalibrationDataQMC qmcCalib;
CalibrationDataBMP bmpCalib;
CalibrationDataBNO bnoCalib;
CalibrationDataBME bmeCalib;

StructInitSensor initSensor;
StructCalibSensor calibSensor;

Preferences preferences;

bool offsetsLoaded = false;
bool offsetsSaved = false;

void InitLedBuzzerActuators(){
ledcSetup(RED_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcSetup(GREEN_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcSetup(BLUE_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcSetup(BUZZER_CHANNEL, 2000, 8); 

    ledcAttachPin(LED_RED_PIN, RED_CHANNEL);
    ledcAttachPin(LED_GREEN_PIN, GREEN_CHANNEL);
    ledcAttachPin(LED_BLUE_PIN, BLUE_CHANNEL);
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
    
    pinMode(ACTUATOR1_PIN, OUTPUT);
    pinMode(ACTUATOR2_PIN, OUTPUT);
}

void InitMPU6050(){
    Wire.begin();
    byte status = mpu.begin();
    if (status != 0) {
        CriticalErrorSensor("MPU6050 initialization failed");
        initSensor.initMPU = 0;
    } else {
        println("MPU6050 sensor initialized successfully");
        initSensor.initMPU = 1;
    };
}
void InitQMC5883L(){
    Wire.beginTransmission(0x68); 
    Wire.write(0x37);
    Wire.write(0x02); 
    if (Wire.endTransmission() != 0) {
        CriticalErrorSensor("Failed to open MPU6050 Bypass");
        initSensor.initQMC = 0;
        return;
    }

    Wire.beginTransmission(0x0D); 
    Wire.write(0x09); 
    Wire.write(0x1D); 
    Wire.endTransmission();

    Wire.beginTransmission(0x0D);
    Wire.write(0x0B);
    Wire.write(0x01); 
    
    if (Wire.endTransmission() != 0) {
        CriticalErrorSensor("QMC5883L not found on 0x0D");
        initSensor.initQMC = 0;
    } else {
        println("QMC5883L initialized successfully");
        initSensor.initQMC = 1;
    }
}
void InitBMP180(){
    Wire.begin();
    if (!bmp.begin()) {
	    CriticalErrorSensor("BMP180 initialization failed");
        initSensor.initBMP = 0;
    } else {
        println("BMP180 sensor initialized successfully");
        initSensor.initBMP = 1;
    };
}
void InitBNO055() {
    if (!bno.begin()) {
        CriticalErrorSensor("BNO055 initialization failed");
        initSensor.initBNO = 0;
    } else {
        println("BNO055 sensor initialized successfully");
        initSensor.initBNO = 1;
    };
}
void InitBME280() {
    if (!bme.begin()) {
        CriticalErrorSensor("BME280 initialization failed");
        initSensor.initBME = 0;
    } else {
        println("BME280 sensor initialized successfully");
        initSensor.initBME = 1;
    }
}
void InitUblox() {
    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, UBLOX_RX, UBLOX_TX);

    unsigned long startTime = millis();
    bool GPSInitialized = false;
    while (gpsSerial.available() > 0) {
        gpsSerial.read();
    }
    while (millis() - startTime < 3000) {
        if (gpsSerial.available() > 0) {
            char c = gpsSerial.read(); 
            if (c == '$') { 
                GPSInitialized = true; 
                break;
            }
        }
    }
    if (!GPSInitialized) {
        CriticalErrorSensor("Ublox initialization failed");
        initSensor.initGPS = 0;
    } else {
        println("Ublox sensor initialized successfully");
        initSensor.initGPS = 1;
    }
}


void CalibrateSensors() {
    float gSumX = 0, gSumY = 0, gSumZ = 0;
    float aSumX = 0, aSumY = 0, aSumZ = 0;
    float tSum = 0;
    float pSumBME = 0;
    float pSumBMP = 0;
    bool GPSConected = false;
    int numReadings = 0;
    const int MAX_MU = 1000;
    uint32_t previousCalibMilis = millis();

    // Data collection loop for calibration
    println("Calibration loop.");
    while (numReadings < MAX_MU) {
        uint32_t calibMilis = millis();
        if (calibMilis - previousCalibMilis >= 150) {
            previousCalibMilis = calibMilis;

            byte status = mpu.begin();
            if (status == 0) {
                mpu.update();
                
                gSumX += mpu.getGyroX();
                gSumY += mpu.getGyroY();
                gSumZ += mpu.getGyroZ();
                aSumX += mpu.getAccX();
                aSumY += mpu.getAccY();
                aSumZ += mpu.getAccZ();
                tSum  += mpu.getTemp();
            }
            pSumBME += bme.readPressure();
            pSumBMP += bmp.readPressure();

            numReadings++;
        }
        if (numReadings == 1000){
            calibSensor.calibMPU = 1;
            calibSensor.calibBME = 1;
            calibSensor.calibBMP =1;
        }
    }

    // MPU6050 calibration
    mpuCalib.mpuGyroBiasX = gSumX / float(numReadings);
    mpuCalib.mpuGyroBiasY = gSumY / float(numReadings);
    mpuCalib.mpuGyroBiasZ = gSumZ / float(numReadings);
    mpuCalib.mpuAccBiasX = aSumX / float(numReadings);
    mpuCalib.mpuAccBiasY = aSumY / float(numReadings);
    mpuCalib.mpuAccBiasZ = aSumZ / float(numReadings) - 1;

    // Temperature calibration
    mpuCalib.tempRef = tSum / float(numReadings);
    mpuCalib.gyroTCO = 0.5; 
    mpuCalib.accTCO = 0.0015;

    // BMP180 calibration
    bmpCalib.bmpPresRef = pSumBMP / float(numReadings);

    // BME280 calibration
    bmeCalib.bmePresRef = pSumBME / float(numReadings);

    // BNO055 calibration
    ///bno.getCalibration(&sys, &gyro, &acc, &mag);

    // GPS connection check
    gpsSerial.begin(GPS_BAUD,SERIAL_8N1,UBLOX_RX,UBLOX_TX);
    while(!GPSConected){
        while (gpsSerial.available() > 0) {
            gps.encode(gpsSerial.read());
        }
        if (!GPSConected && gps.location.isValid() && gps.satellites.value() > 3) {
            GPSConected = true;
            calibSensor.calibGPS = 1;
        }
    }
}

void CalibratMagnetometer() {
    float magMin[3] = {32767, 32767, 32767};
    float magMax[3] = {-32768, -32768, -32768};

    uint32_t previousCalibMagMillis = 0;
    uint32_t startTime = millis();

    bool calibrated = false;

     while (!calibrated) {

        uint32_t calibMag = millis();

        if (calibMag - previousCalibMagMillis >= 500) {

            previousCalibMagMillis = calibMag;

            Wire.beginTransmission(0x0D);
            Wire.write(0x00);
            Wire.endTransmission();

            Wire.requestFrom(0x0D, 6);

            if (Wire.available() == 6) {

                int16_t rawX = (int16_t)(Wire.read() | (Wire.read() << 8));
                int16_t rawY = (int16_t)(Wire.read() | (Wire.read() << 8));
                int16_t rawZ = (int16_t)(Wire.read() | (Wire.read() << 8));

                // ===== UPDATE MIN =====
                if (rawX < magMin[0]) magMin[0] = rawX;
                if (rawY < magMin[1]) magMin[1] = rawY;
                if (rawZ < magMin[2]) magMin[2] = rawZ;

                // ===== UPDATE MAX =====
                if (rawX > magMax[0]) magMax[0] = rawX;
                if (rawY > magMax[1]) magMax[1] = rawY;
                if (rawZ > magMax[2]) magMax[2] = rawZ;

            }
        }

        if (millis() - startTime >= 30000) {
            calibrated = true;
            calibSensor.calibQMC = 1;
        }

        delay(1);
    }

    qmcCalib.qmcMagOffsetX = (magMax[0] + magMin[0]) / 2.0f;
    qmcCalib.qmcMagOffsetY = (magMax[1] + magMin[1]) / 2.0f;
    qmcCalib.qmcMagOffsetZ = (magMax[2] + magMin[2]) / 2.0f;

    float radioX = (magMax[0] - magMin[0]) / 2.0f;
    float radioY = (magMax[1] - magMin[1]) / 2.0f;
    float radioZ = (magMax[2] - magMin[2]) / 2.0f;

    if (radioX == 0) radioX = 1;
    if (radioY == 0) radioY = 1;
    if (radioZ == 0) radioZ = 1;

    float radioPromedio = (radioX + radioY + radioZ) / 3.0f;

    qmcCalib.qmcMagScaleX = radioPromedio / radioX;
    qmcCalib.qmcMagScaleY = radioPromedio / radioY;
    qmcCalib.qmcMagScaleZ = radioPromedio / radioZ;

}


void ReadMPU6050(){
    mpu.update();
    mpuData.timestamp = millis();

    mpuData.MPU_temp = mpu.getTemp();
    float deltaTemp = mpuData.MPU_temp - mpuCalib.tempRef;

    float ax_g = mpu.getAccX() - mpuCalib.mpuAccBiasX - (mpuCalib.accTCO * deltaTemp);
    float ay_g = mpu.getAccY() - mpuCalib.mpuAccBiasY - (mpuCalib.accTCO * deltaTemp);
    float az_g = mpu.getAccZ() - mpuCalib.mpuAccBiasZ - (mpuCalib.accTCO * deltaTemp);

    mpuData.MPU_ax = ax_g * 9.80665;
    mpuData.MPU_ay = ay_g * 9.80665;
    mpuData.MPU_az = az_g * 9.80665;

    float gx_deg = mpu.getGyroX() - mpuCalib.mpuGyroBiasX - (mpuCalib.gyroTCO * deltaTemp);
    float gy_deg = mpu.getGyroY() - mpuCalib.mpuGyroBiasY - (mpuCalib.gyroTCO * deltaTemp);
    float gz_deg = mpu.getGyroZ() - mpuCalib.mpuGyroBiasZ - (mpuCalib.gyroTCO * deltaTemp);

    mpuData.MPU_gx = gx_deg * 0.0174533;
    mpuData.MPU_gy = gy_deg * 0.0174533;
    mpuData.MPU_gz = gz_deg * 0.0174533;
}

void ReadQMC5883L(){
    Wire.beginTransmission(0x0D);
    Wire.write(0x00);
    Wire.endTransmission();
    Wire.requestFrom(0x0D, 6);

    if (Wire.available() == 6) {
        int16_t rawX = (int16_t)(Wire.read() | (Wire.read() << 8));
        int16_t rawY = (int16_t)(Wire.read() | (Wire.read() << 8));
        int16_t rawZ = (int16_t)(Wire.read() | (Wire.read() << 8));

        qmcData.timestamp = millis();
        qmcData.QMC_mx = (rawX / 3000.0f) * 1e-4f;
        qmcData.QMC_my = (rawY / 3000.0f) * 1e-4f;
        qmcData.QMC_mz = (rawZ / 3000.0f) * 1e-4f;

/*
        float correctedX = (rawX - qmcCalib.qmcMagOffsetX) * qmcCalib.qmcMagScaleX;
        float correctedY = (rawY - qmcCalib.qmcMagOffsetY) * qmcCalib.qmcMagScaleY;
        float correctedZ = (rawZ - qmcCalib.qmcMagOffsetZ) * qmcCalib.qmcMagScaleZ;

        qmcData.timestamp = millis();

        qmcData.QMC_mx = (correctedX / 3000.0f) * 1e-4f;
        qmcData.QMC_my = (correctedY / 3000.0f) * 1e-4f;
        qmcData.QMC_mz = (correctedZ / 3000.0f) * 1e-4f;
*/
    }
}
void ReadBNO055() {
    // Code to read data from BNO055 sensor
    bnoData.timestamp = millis();
    imu::Vector<3> lin_accel = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
    bnoData.BNO_ax = lin_accel.x();
    bnoData.BNO_ay = lin_accel.y();
    bnoData.BNO_az = lin_accel.z();
    imu::Vector<3> gyro = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
    bnoData.BNO_gx = gyro.x() * 0.0174533;
    bnoData.BNO_gy = gyro.y() * 0.0174533;
    bnoData.BNO_gz = gyro.z() * 0.0174533;
    imu::Vector<3> mag = bno.getVector(Adafruit_BNO055::VECTOR_MAGNETOMETER);
    bnoData.BNO_mx = mag.x() * 1e-6;
    bnoData.BNO_my = mag.y() * 1e-6;
    bnoData.BNO_mz = mag.z() * 1e-6;
     imu::Quaternion quat = bno.getQuat();
    bnoData.BNO_qw = quat.w();
    bnoData.BNO_qx = quat.x();
    bnoData.BNO_qy = quat.y();
    bnoData.BNO_qz = quat.z();
}
void ReadBMP180(){
    float pressurePad1 = bmpCalib.bmpPresRef;
    bmpData.timestamp = millis();
    bmpData.temp = bmp.readTemperature();
    bmpData.pressure = bmp.readPressure();
    bmpData.altitude = bmp.readAltitude(pressurePad1);

}
void ReadBME280() {
    float pressurePad2 = bmeCalib.bmePresRef / 100.0f; 
    bmeData.timestamp = millis();
    bmeData.temp = bme.readTemperature();
    bmeData.humidity = bme.readHumidity();
    bmeData.pressure = bme.readPressure();
    bmeData.altitude = bme.readAltitude(pressurePad2);
}
void ReadUblox() {
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

void OpenActuators1Voltage() {
    digitalWrite(ACTUATOR1_PIN, HIGH);
    digitalWrite(LED_GREEN_PIN, HIGH);
}
void CloseActuators1Voltage() {
    digitalWrite(ACTUATOR1_PIN, LOW);
    digitalWrite(LED_GREEN_PIN, LOW);
}
void OpenActuators2Voltage() {
    digitalWrite(ACTUATOR2_PIN, HIGH);
    digitalWrite(LED_GREEN_PIN, HIGH);
}
void CloseActuators2Voltage() {
    digitalWrite(ACTUATOR2_PIN, LOW);
    digitalWrite(LED_GREEN_PIN, LOW);
}