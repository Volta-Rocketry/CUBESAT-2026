# Objective
In this part, the data and their physical units are defined, enabling consistent information exchange among the participants. This avoids variable discrepancies and interpretation errors, regardless of the hardware. Additionally, all sensors will use the exact units established in this document.
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