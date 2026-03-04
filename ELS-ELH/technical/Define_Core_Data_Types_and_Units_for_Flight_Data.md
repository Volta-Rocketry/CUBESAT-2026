# Objective
In this part, the data and their physical units are defined, establishing data structures that enable consistent information exchange between roles, avoiding differences in variables or interpretation errors, without depending on the hardware.
# Data Standards and Physical Units
* **Acceleration:** Meters per second squared ($m/s^2$). Variable: `accel`
  Type: `float`
* **Linear Velocity:** Meters per second ($m/s$). Variable: `linVel`
  Type: `float`
* **Angular Velocity:** Radians per second ($rad/s$). Variable: `angVel` Type: `float`
* **Magnetometer:** microteslas ($\mu T$). Variable: `mag` Type: `float`
* **Pressure:** Pascals ($Pa$). Variable: `pressure` Type: `float`
* **Altitude:** Meters ($m$). Variable: `altitude` Type: `float`
* **Temperature:** Degrees Celsius ($^\circ C$). Variable: `temp` Type: `float`
* **Humidity:** Percentage (%). Variable: `humidity` Type: `float`
* **Time:** Seconds ($s$) since system startup. Variable: `timestamp` Type: `float`
# Data Structures

* **MPU9250 Structure**
```cpp
struct StructMPU9250{
    float timestamp;  // Timestamp
    // Acceleration
    float MPU_ax;     // MPU9250 acceleration on the X axis
    float MPU_ay;     // MPU9250 acceleration on the Y axis
    float MPU_az;     // MPU9250 acceleration on the Z axis
    // Angular velocity
    float MPU_gx;     // MPU9250 angular velocity on the X axis
    float MPU_gy;     // MPU9250 angular velocity on the Y axis
    float MPU_gz:     // MPU9250 angular velocity on the Z axis
    // Magnetic field
    float MPU_mx;     // MPU9250 magnetic field on the X axis
    float MPU_my;     // MPU9250 magnetic field on the Y axis
    float MPU_mz;     // MPU9250 magnetic field on the Z axis
}
```
* **BNO055 Structure**
```cpp
struct StructBNO055{
    float timestamp;  // Timestamp
    // Acceleration
    float BNO_ax;     // BNO055 acceleration on the X axis
    float BNO_ay;     // BNO055 acceleration on the Y axis
    float BNO_az;     // BNO055 acceleration on the Z axis
    // Angular velocity
    float BNO_gx;     // BNO055 angular velocity on the X axis
    float BNO_gy;     // BNO055 angular velocity on the Y axis
    float BNO_gz:     // BNO055 angular velocity on the Z axis
    // Magnetic field
    float BNO_mx;     // BNO055 magnetic field on the X axis
    float BNO_my;     // BNO055 magnetic field on the Y axis
    float BNO_mz;     // BNO055 magnetic field on the Z axis
}
```
* **BME280 Structure**
```cpp
struct StructBME280 {
    float temp;       // Temperature
    float humidity;   // Humidity
    float pressure;   // Pressure
    float altitude;   // Altitude
    float timestamp;  // Timestamp
};
```
* **Ublox Structure**
```cpp
struct StructUblox {
    float timestamp;  // Timestamp
    float lat;        // Latitude
    float lon;        // Longitude
    float alt;        // Altitude
    float speed;      // Speed
};
```
# Processed Data
This section defines variables for processed data to ensure that only validated signals are used, reducing noise propagation and minimizing errors.



**State Estimations:**
Stores estimated position and velocity obtained from acceleration measurements.
```cpp
struct StructEstimated {
  // Estimated position (m)
    float px; // X axis
    float py; // Y axis
    float pz; // Z axis
  // Estimated velocity (m/s)
    float sx; // X axis
    float sy; // Y axis
    float sz; // Z axis
    float timestamp; // Timestamp
};
```
**Digital Filtering:**
Stores values smoothed using filtering algorithms.
```cpp
struct filtered_data {
    float accelF[3]; // [ax, ay, az] filtered acceleration
    float angVelF[3];  // [gx, gy, gz] filtered velocity
    float altF;      // Filtered altitude
    float timestamp; // Timestamp
};
```