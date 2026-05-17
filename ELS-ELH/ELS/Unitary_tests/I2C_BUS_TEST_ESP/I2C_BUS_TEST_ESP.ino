/*
I2C device found at address 0x0D
I2C device found at address 0x20 -> PCF8574
I2C device found at address 0x28
I2C device found at address 0x68
I2C device found at address 0x77
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h> 
#include <Adafruit_BNO055.h>

#include <Adafruit_PCF8574.h>

#include <Adafruit_BNO055.h>

#include <Adafruit_BMP085.h>
#include <DFRobot_QMC5883.h>
#include <MPU6050_light.h>

MPU6050 mpu(Wire);
Adafruit_BMP085 bmp;
Adafruit_BNO055 bno;
Adafruit_PCF8574 pcf;

#define SCL_PIN 22
#define SDA_PIN 21

#define PCF_ADDRESS 0x20

#define PCF_P0 0
#define PCF_P1 1
#define PCF_P2 2
#define PCF_P3 3
#define PCF_P4 4
#define PCF_P5 5
#define PCF_P6 6
#define PCF_P7 7

#define PCF_ACTIVATION_DELAY 5000

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  delay(10);
  Serial.println("Serial Monitor Intialized");

  // Init I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  // Init PCF PCF8574
  if (!pcf.begin(PCF_ADDRESS, &Wire)) {
    Serial.println("Error while intializing PCF8574");
    Serial.print("Execution Halted");
    while (true)
      ;
  } else {
    Serial.println("PCF8574 Intialized Succesfully");

    // Initialize PCG8574 pins
    pcf.pinMode(PCF_P1, OUTPUT);
    pcf.pinMode(PCF_P2, OUTPUT);
    pcf.pinMode(PCF_P3, OUTPUT);
    pcf.pinMode(PCF_P4, OUTPUT);
    pcf.pinMode(PCF_P5, OUTPUT);
    pcf.pinMode(PCF_P6, OUTPUT);
    pcf.pinMode(PCF_P7, OUTPUT);

    pcf.digitalWrite(PCF_P1, LOW);
    pcf.digitalWrite(PCF_P2, LOW);
    pcf.digitalWrite(PCF_P3, LOW);
    pcf.digitalWrite(PCF_P4, LOW);
    pcf.digitalWrite(PCF_P5, LOW);
    pcf.digitalWrite(PCF_P6, LOW);
    pcf.digitalWrite(PCF_P7, LOW);
  }

  // Init BNO055
  if (!bno.begin()) {
    Serial.println("Error while intializing BNO055");
    Serial.print("Execution Halted");
    while (true)
      ;
  } else {
    Serial.println("BNO055 Intialized Succesfully");
  }

  // Init MPU6050
  byte status = mpu.begin();
  if (status != 0) {
    Serial.println("Error while intializing MPU6050");
    Serial.print("Execution Halted");
    while (true)
      ;
  } else {
    Serial.println("MPU6050 Intialized Succesfully");
  }

  // Init QMC5883L
  Wire.beginTransmission(0x68);
  Wire.write(0x37);
  Wire.write(0x02);
  if (Wire.endTransmission() != 0) {
    Serial.println("Failed to open MPU6050 Bypass");
    Serial.print("Execution Halted");
    while (true)
      ;
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
    Serial.print("Execution Halted");
    while (true)
      ;
  } else {
    Serial.println("QMC5883L initialized successfully.");
  }

  // Init BMP180
  if (!bmp.begin()) {
    Serial.println("BMP180 initialization failed");
    Serial.print("Execution Halted");
    while (true)
      ;
  } else {
    Serial.println("BMP180 sensor initialized successfully.");
  };
}

void loop() {
  Serial.println(" ----- RETRIEVING DATA FROM BNO055 -----");
  imu::Vector<3> lin_accel = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
  imu::Vector<3> gyro = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
  imu::Vector<3> mag = bno.getVector(Adafruit_BNO055::VECTOR_MAGNETOMETER);
  imu::Quaternion quat = bno.getQuat();

  // Linear Acceleration
  Serial.print("Ax: ");
  Serial.print(lin_accel.x());
  Serial.print(", Ay: ");
  Serial.print(lin_accel.y());
  Serial.print(", Az: ");
  Serial.println(lin_accel.z());

  // Gyroscope
  Serial.print("Gx: ");
  Serial.print(gyro.x());
  Serial.print(", Gy: ");
  Serial.print(gyro.y());
  Serial.print(", Gz: ");
  Serial.println(gyro.z());

  // Magnetometer
  Serial.print("Mx: ");
  Serial.print(mag.x());
  Serial.print(", My: ");
  Serial.print(mag.y());
  Serial.print(", Mz: ");
  Serial.println(mag.z());

  // Quaternion
  Serial.print("Qw: ");
  Serial.print(quat.w());
  Serial.print(", Qx: ");
  Serial.print(quat.x());
  Serial.print(", Qy: ");
  Serial.print(quat.y());
  Serial.print(", Qz: ");
  Serial.println(quat.z());

  Serial.println(" ----- RETRIEVING DATA FROM MPU6050 -----");
  // Linear Acceleration
  Serial.print("Ax: ");
  Serial.print(mpu.getAccX());
  Serial.print(", Ay: ");
  Serial.print(mpu.getAccY());
  Serial.print(", Az: ");
  Serial.println(mpu.getAccZ());

  // Gyroscope
  Serial.print("Gx: ");
  Serial.print(mpu.getGyroX());
  Serial.print(", Gy: ");
  Serial.print(mpu.getGyroY());
  Serial.print(", Gz: ");
  Serial.println(mpu.getGyroZ());

  Serial.println(" ----- RETRIEVING DATA FROM BMP180 -----");
  Serial.print("Temp: ");
  Serial.print(bmp.readTemperature());
  Serial.print(", Press: ");
  Serial.print(bmp.readPressure());
  Serial.print(", Alt: ");
  Serial.println(bmp.readAltitude());

  Serial.println(" ----- RETRIEVING DATA FROM QMC5883L -----");
  Wire.beginTransmission(0x0D);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(0x0D, 6);

  if (Wire.available() == 6) {
    int16_t rawX = (int16_t)(Wire.read() | (Wire.read() << 8));
    int16_t rawY = (int16_t)(Wire.read() | (Wire.read() << 8));
    int16_t rawZ = (int16_t)(Wire.read() | (Wire.read() << 8));

    Serial.print("Mx: ");
    Serial.print((rawX / 3000.0f) * 1e-4f);
    Serial.print(", My: ");
    Serial.print((rawY / 3000.0f) * 1e-4f);
    Serial.print(", Mz: ");
    Serial.println((rawZ / 3000.0f) * 1e-4f);
  } else {
    Serial.println("No Data available");
  }

  delay(1000);

  Serial.println("Activatin PCF PIN 0...");
  pcf.digitalWrite(PCF_P0, HIGH);
  pcf.digitalWrite(PCF_P1, LOW);
  pcf.digitalWrite(PCF_P2, LOW);
  pcf.digitalWrite(PCF_P3, LOW);
  pcf.digitalWrite(PCF_P4, LOW);
  pcf.digitalWrite(PCF_P5, LOW);
  pcf.digitalWrite(PCF_P6, LOW);
  pcf.digitalWrite(PCF_P7, LOW);
  delay(PCF_ACTIVATION_DELAY);

  Serial.println("Activatin PCF PIN 1...");
  pcf.digitalWrite(PCF_P0, LOW);
  pcf.digitalWrite(PCF_P1, HIGH);
  pcf.digitalWrite(PCF_P2, LOW);
  pcf.digitalWrite(PCF_P3, LOW);
  pcf.digitalWrite(PCF_P4, LOW);
  pcf.digitalWrite(PCF_P5, LOW);
  pcf.digitalWrite(PCF_P6, LOW);
  pcf.digitalWrite(PCF_P7, LOW);
  delay(PCF_ACTIVATION_DELAY);

  Serial.println("Activating PCF PIN 2...");
  pcf.digitalWrite(PCF_P0, LOW);
  pcf.digitalWrite(PCF_P1, LOW);
  pcf.digitalWrite(PCF_P2, HIGH);
  pcf.digitalWrite(PCF_P3, LOW);
  pcf.digitalWrite(PCF_P4, LOW);
  pcf.digitalWrite(PCF_P5, LOW);
  pcf.digitalWrite(PCF_P6, LOW);
  pcf.digitalWrite(PCF_P7, LOW);
  delay(PCF_ACTIVATION_DELAY);

  Serial.println("Activating PCF PIN 3...");
  pcf.digitalWrite(PCF_P0, LOW);
  pcf.digitalWrite(PCF_P1, LOW);
  pcf.digitalWrite(PCF_P2, LOW);
  pcf.digitalWrite(PCF_P3, HIGH);
  pcf.digitalWrite(PCF_P4, LOW);
  pcf.digitalWrite(PCF_P5, LOW);
  pcf.digitalWrite(PCF_P6, LOW);
  pcf.digitalWrite(PCF_P7, LOW);
  delay(PCF_ACTIVATION_DELAY);

  Serial.println("Activating PCF PIN 4...");
  pcf.digitalWrite(PCF_P0, LOW);
  pcf.digitalWrite(PCF_P1, LOW);
  pcf.digitalWrite(PCF_P2, LOW);
  pcf.digitalWrite(PCF_P3, LOW);
  pcf.digitalWrite(PCF_P4, HIGH);
  pcf.digitalWrite(PCF_P5, LOW);
  pcf.digitalWrite(PCF_P6, LOW);
  pcf.digitalWrite(PCF_P7, LOW);
  delay(PCF_ACTIVATION_DELAY);

  Serial.println("Activating PCF PIN 5...");
  pcf.digitalWrite(PCF_P0, LOW);
  pcf.digitalWrite(PCF_P1, LOW);
  pcf.digitalWrite(PCF_P2, LOW);
  pcf.digitalWrite(PCF_P3, LOW);
  pcf.digitalWrite(PCF_P4, LOW);
  pcf.digitalWrite(PCF_P5, HIGH);
  pcf.digitalWrite(PCF_P6, LOW);
  pcf.digitalWrite(PCF_P7, LOW);
  delay(PCF_ACTIVATION_DELAY);

  Serial.println("Activating PCF PIN 6...");
  pcf.digitalWrite(PCF_P0, LOW);
  pcf.digitalWrite(PCF_P1, LOW);
  pcf.digitalWrite(PCF_P2, LOW);
  pcf.digitalWrite(PCF_P3, LOW);
  pcf.digitalWrite(PCF_P4, LOW);
  pcf.digitalWrite(PCF_P5, LOW);
  pcf.digitalWrite(PCF_P6, HIGH);
  pcf.digitalWrite(PCF_P7, LOW);
  delay(PCF_ACTIVATION_DELAY);

  Serial.println("Activating PCF PIN 7...");
  pcf.digitalWrite(PCF_P0, LOW);
  pcf.digitalWrite(PCF_P1, LOW);
  pcf.digitalWrite(PCF_P2, LOW);
  pcf.digitalWrite(PCF_P3, LOW);
  pcf.digitalWrite(PCF_P4, LOW);
  pcf.digitalWrite(PCF_P5, LOW);
  pcf.digitalWrite(PCF_P6, LOW);
  pcf.digitalWrite(PCF_P7, HIGH);
  delay(PCF_ACTIVATION_DELAY);

  Serial.println("Deactivating");
  pcf.digitalWrite(PCF_P0, LOW);
  pcf.digitalWrite(PCF_P1, LOW);
  pcf.digitalWrite(PCF_P2, LOW);
  pcf.digitalWrite(PCF_P3, LOW);
  pcf.digitalWrite(PCF_P4, LOW);
  pcf.digitalWrite(PCF_P5, LOW);
  pcf.digitalWrite(PCF_P6, LOW);
  pcf.digitalWrite(PCF_P7, LOW);
  delay(PCF_ACTIVATION_DELAY);
}
