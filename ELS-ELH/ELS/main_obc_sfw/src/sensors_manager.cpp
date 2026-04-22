#include "sensors_manager.h"

#include "constants.h"
#include "error_warning.h"
#include "signals.h"
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_BME280.h>
#include <Adafruit_PCF8574.h>
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

extern Adafruit_PCF8574 pcf;

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
        SerialBT.println("PCF8574 pins configured successfully.");
    }
}

// Sensor initialization function
void InitMPU6050(){
    byte status = mpu.begin();
    if (status != 0) {
        CriticalErrorSensor("MPU6050 initialization failed");
    } else {
        Serial.println("MPU6050 sensor initialized successfully.");
        SerialBT.println("MPU6050 sensor initialized successfully.");
    };
}
void InitQMC5883L(){
    Wire.beginTransmission(0x68); 
    Wire.write(0x37);
    Wire.write(0x02); 
    if (Wire.endTransmission() != 0) {
        CriticalErrorSensor("Failed to open MPU6050 Bypass");
        SerialBT.println("Failed to open MPU6050 Bypass");
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
    } else {
        Serial.println("QMC5883L initialized successfully.");
        SerialBT.println("QMC5883L initialized successfully.");
    }
}
void InitBMP180(){
    if (!bmp.begin()) {
	    CriticalErrorSensor("BMP180 initialization failed");
    } else {
        Serial.println("BMP180 sensor initialized successfully.");
        SerialBT.println("BMP180 sensor initialized successfully.");
    };
}
void InitBNO055() {
    // Code to initialize BNO055 sensor
    if (!bno.begin()) {
        CriticalErrorSensor("BNO055 initialization failed");
    } else {
        Serial.println("BNO055 sensor initialized successfully.");
        SerialBT.println("BNO055 sensor initialized successfully.");
    };
}
void InitBME280() {
    // Code to initialize BME280 sensor
    if (!bme.begin()) {
        CriticalErrorSensor("BME280 initialization failed");
    } else {
        Serial.println("BME280 sensor initialized successfully.");
        SerialBT.println("BME280 sensor initialized successfully.");
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
    } else {
        Serial.println("Ublox sensor initialized successfully.");
        SerialBT.println("Ublox sensor initialized successfully.");
    }
}
// Sensor calibration functions
void CalibrateSensors() {
    // Variables temporales para el proceso de calibración
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
    Serial.println("Calibration loop");
    SerialBT.println("Calibration loop");
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
        SerialBT.print("Numbers of readings: ");
        SerialBT.println(numReadings);
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
    mpuCalib.gyroTCO = 0.5; // 0.5 deg/s 
    mpuCalib.accTCO = 0.0015;  // 1.5 mg = 0.0015 G

    // BMP180 calibration
    bmpCalib.bmpPresRef = pSumBMP / float(numReadings);
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

/*
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
    */


    // BME280 calibration
    bmeCalib.bmePresRef = pSumBME / float(numReadings);
    // numCalib = 1;

    // GPS connection check
    gpsSerial.begin(GPS_BAUD,SERIAL_8N1,UBLOX_RX,UBLOX_TX);
    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
    }
    if (!GPSConected && gps.location.isValid() && gps.satellites.value() > 3) {
        GPSConected = true;
        // numCalib = 2;
    }
}

void CalibratMagnetometer() {
    float magMin[3] = {32767, 32767, 32767};
    float magMax[3] = {-32768, -32768, -32768};

    uint32_t previousCalibMagMillis = millis();
    bool calibrated = false;
    while (!calibrated) {
        uint32_t calibMagSec = millis();
        if (calibMagSec - previousCalibMagMillis >= 500) {
            previousCalibMagMillis = calibMagSec;

            Wire.beginTransmission(0x0D);
            Wire.write(0x00);
            Wire.endTransmission();
            Wire.requestFrom(0x0D, 6);

            if (Wire.available() == 6) {

                float mx = (int16_t)(Wire.read() | (Wire.read() << 8));
                float my = (int16_t)(Wire.read() | (Wire.read() << 8));
                float mz = (int16_t)(Wire.read() | (Wire.read() << 8));

                if (mx < magMin[0]) magMin[0] = mx;
                if (my < magMin[1]) magMin[1] = my;
                if (mz < magMin[2]) magMin[2] = mz;

                if (mx > magMax[0]) magMax[0] = mx;
                if (my > magMax[1]) magMax[1] = my;
                if (mz > magMax[2]) magMax[2] = mz;
            }
            if (previousCalibMagMillis == 30000){
                calibrated = true;
            }
        }
    }
    qmcCalib.qmcMagOffsetX = (magMax[0] + magMin[0]) / 2.0;
    qmcCalib.qmcMagOffsetY = (magMax[1] + magMin[1]) / 2.0;
    qmcCalib.qmcMagOffsetZ = (magMax[2] + magMin[2]) / 2.0;

    float radioX = (magMax[0] - magMin[0]) / 2.0;
    float radioY = (magMax[1] - magMin[1]) / 2.0;
    float radioZ = (magMax[2] - magMin[2]) / 2.0;

    float radioPromedio = (radioX + radioY + radioZ) / 3.0;

    qmcCalib.qmcMagScaleX = radioPromedio / radioX;
    qmcCalib.qmcMagScaleY = radioPromedio / radioY;
    qmcCalib.qmcMagScaleZ = radioPromedio / radioZ;
}


// Sensor reading functions
void ReadMPU6050(){
    mpu.update();
    mpuData.timestamp = millis();

    mpuData.MPU_temp = mpu.getTemp();
    float deltaTemp = mpuData.MPU_temp - mpuCalib.tempRef;

    mpuData.MPU_ax = mpu.getAccX();
    mpuData.MPU_ay = mpu.getAccY(); 
    mpuData.MPU_az = mpu.getAccZ();
    
    mpuData.MPU_gx = mpu.getGyroX();
    mpuData.MPU_gy = mpu.getGyroY();
    mpuData.MPU_gz = mpu.getGyroZ();

    // Codigo calibrado y en m/s2 y rad/s
    /*
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
    */
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
        float correctedX = (rawX - hmcCalib.qmcMagOffsetX) * hmcCalib.qmcMagScaleX;
        float correctedY = (rawY - hmcCalib.qmcMagOffsetY) * hmcCalib.qmcMagScaleY;
        float correctedZ = (rawZ - hmcCalib.qmcMagOffsetZ) * hmcCalib.qmcMagScaleZ;

        qmcData.timestamp = millis();

        qmcData.QMC_mx = (correctedX / 3000.0f) * 1e-4f;
        qmcData.QMC_my = (correctedY / 3000.0f) * 1e-4f;
        qmcData.QMC_mz = (correctedZ / 3000.0f) * 1e-4f;
*/
    }
}
void ReadBMP180(){
    float pressurePad1 = bmpCalib.bmpPresRef / 100.0;
    bmpData.timestamp = millis();
    bmpData.temp = bmp.readTemperature();
    bmpData.pressure = bmp.readPressure();
    bmpData.altitude = bmp.readAltitude(pressurePad1);

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
void ReadBME280() {
    // Code to read data from BME sensor
    float pressurePad2 = bmeCalib.bmePresRef / 100.0; 
    bmeData.timestamp = millis();
    bmeData.temp = bme.readTemperature();
    bmeData.humidity = bme.readHumidity();
    bmeData.pressure = bme.readPressure();
    bmeData.altitude = bme.readAltitude(pressurePad2);
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
void OpenActuators1Voltage() {
    // Code to send voltage to actuators
    pcf.digitalWrite(ACTUATOR1_PIN, HIGH);
    pcf.digitalWrite(LED_GREEN_PIN, HIGH);
}
void CloseActuators1Voltage() {
    // Code to stop voltage to actuators
    pcf.digitalWrite(ACTUATOR1_PIN, LOW);
    pcf.digitalWrite(LED_GREEN_PIN, LOW);
}
void OpenActuators2Voltage() {
    // Code to send voltage to actuators
    pcf.digitalWrite(ACTUATOR2_PIN, HIGH);
    pcf.digitalWrite(LED_GREEN_PIN, HIGH);
}
void CloseActuators2Voltage() {
    // Code to stop voltage to actuators
    pcf.digitalWrite(ACTUATOR2_PIN, LOW);
    pcf.digitalWrite(LED_GREEN_PIN, LOW);
}