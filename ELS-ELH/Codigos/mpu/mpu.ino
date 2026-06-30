#include <MPU6050_light.h>
#include <Adafruit_BMP085.h>
#include <DFRobot_QMC5883.h>
#include <Adafruit_BNO055.h>

MPU6050 mpu(Wire);
Adafruit_BMP085 bmp;
Adafruit_BNO055 bno;

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

struct StructBMP180{
    uint32_t timestamp;  ///< Reading timestamp in ms
    float temp;          ///< Temperature in Celsius degrees
    float pressure;      ///< Atmospheric pressure in Pa
    float altitude;      ///< Calculated altitude in meters
};
struct StructQMC5883L{
    uint32_t timestamp; ///< Reading timestamp in ms
    float QMC_mx;       ///< Magnetic field on the X axis
    float QMC_my;       ///< Magnetic field on the Y axis
    float QMC_mz;       ///< Magnetic field on the Z axis
};

StructMPU6050 mpuData; 
StructBMP180 bmpData; 
StructBNO055 bnoData;
StructQMC5883L qmcData;

void InitMPU6050(){
  Wire.begin();
  byte status = mpu.begin();
  if (status != 0) {
      Serial.println("MPU6050 initialization failed");
  } else {
      Serial.println("MPU6050 sensor initialized successfully");
  };
}

void InitBMP180(){
   Wire.begin();
    if (!bmp.begin()) {
	    Serial.println("BMP180 initialization failed");
    } else {
        Serial.println("BMP180 sensor initialized successfully");
    };
}
void InitQMC5883L(){
    Wire.beginTransmission(0x68); 
    Wire.write(0x37);
    Wire.write(0x02); 
    if (Wire.endTransmission() != 0) {
        Serial.println("Failed to open MPU6050 Bypass");
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
        Serial.println("QMC5883L not found on 0x0D");
    } else {
        Serial.println("QMC5883L initialized successfully");
    }
}
void InitBNO055() {
    // Code to initialize BNO055 sensor
    if (!bno.begin()) {
        Serial.println("BNO055 initialization failed");
    } else {
        Serial.println("BNO055 sensor initialized successfully");
    };
}



void ReadMPU6050(){
    mpu.update();
    mpuData.timestamp = millis();

    // Codigo calibrado y en m/s2 y rad/s
    float ax_g = mpu.getAccX();
    float ay_g = mpu.getAccY();
    float az_g = mpu.getAccZ();

    mpuData.MPU_ax = ax_g * 9.80665;
    mpuData.MPU_ay = ay_g * 9.80665;
    mpuData.MPU_az = az_g * 9.80665;

    float gx_deg = mpu.getGyroX();
    float gy_deg = mpu.getGyroY();
    float gz_deg = mpu.getGyroZ();

    mpuData.MPU_gx = gx_deg * 0.0174533;
    mpuData.MPU_gy = gy_deg * 0.0174533;
    mpuData.MPU_gz = gz_deg * 0.0174533;

}

void ReadBMP180(){
    float pressurePad1 = // sea level
    bmpData.timestamp = millis();
    bmpData.temp = bmp.readTemperature();
    bmpData.pressure = bmp.readPressure();
    bmpData.altitude = bmp.readAltitude(pressurePad1);
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

        float correctedX = (rawX);
        float correctedY = (rawY);
        float correctedZ = (rawZ);

        qmcData.timestamp = millis();

        qmcData.QMC_mx = (correctedX / 3000.0f) * 1e-4f;
        qmcData.QMC_my = (correctedY / 3000.0f) * 1e-4f;
        qmcData.QMC_mz = (correctedZ / 3000.0f) * 1e-4f;
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
    
    imu::Vector<3> mag = bno.getVector(Adafruit_BNO055::VECTOR_MAGNETOMETER);
    bnoData.BNO_mx = mag.x() * 1e-6;
    bnoData.BNO_my = mag.y() * 1e-6;
    bnoData.BNO_mz = mag.z() * 1e-6;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  InitMPU6050();
  InitBMP180();
  InitQMC5883L();



}

void loop() {


}
