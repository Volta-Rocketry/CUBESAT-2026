# Cubesat Final Architecture #

## Description ##
The system is based on one electrically identical battery pack that supply two onboard computers with strictly distinct functions, with the MAIN OBC handling all system-level functions, including data acquisition from sensors (GPS, fused IMU, raw IMU, and barometer), data logging to flash memory and an SD card, communications and mode management, distributing validated status information to the CTR OBC and the cameras, and enabling or disabling power delivery to the actuators.
 
The CTR OBC is dedicated exclusively to real-time control and flight dynamics, generating actuator commands based on the received status without managing power or logging.

## Dual OBC Architecture
The **Control OBC** is dedicated exclusively to running control algorithms and robust state estimation. It assumes that actuator power channels are available, and it outputs control signals accordingly. It does not perform power arbitration, logging, or sensor preprocessing. Its operation depends on receiving validated sensor data and system status from MAIN OBC.

The **Main OBC** monitors battery voltages and currents and is responsible for all sensor interfacing and data handling. It acquires raw sensor data, processes and conditions it, runs secondary or complementary state estimation algorithms, and stores data for later retrieval. It also handles non-critical system services such as data logging, Bluetooth communication with an external application. It can issolate the power of the actuators in case of detecting faults on errors in the system. 

Communication between the two OBCs it's given through UART.


## Pin and Interface Budget

### MAIN OBC
| TYPE   | ITEM                       | PINS | COMMENTS |
|--------|----------------------------|------|----------|
| COM    | UART1 TX (CTR OBC)         | 1    | For communication with Control OBC |
| COM    | UART1 RX (CTR OBC)         | 1    | For communication with Control OBC |
| COM    | UART2 TX (CAM)             | 1    | Communication with Cameras Module |
| COM    | UART2 RX (CAM)             | 1    | Communication with Cameras Module |
| COM    | UART3 TX (GPS)             | 1    | For GPS module connection |
| COM    | UART3 RX (GPS)             | 1    | For GPS module connection |
| COM    | SDA I2C1                   | 2    | I2C1 bus definition |
| COM    | SCL I2C1                   | 2    | I2C1 bus definition |
| COM    | SCK SPI1                   | 1    | SPI1 bus definition |
| COM    | MOSI SPI1                  | 1    | SPI1 bus definition |
| COM    | MISO SPI1                  | 1    | SPI1 bus definition |
| COM    | SCK SPI2                   | 1    | SPI2 bus definition |
| COM    | MOSI SPI2                  | 1    | SPI2 bus definition |
| COM    | MISO SPI2                  | 1    | SPI2 bus definition |
| SENSOR | IMU 9DOF Fused             | 0    | Connected to I2C1   |
| SENSOR | IMU 9DOF Raw CS 2          | 1    | Connected to SPI2   |
| SENSOR | Altimeter CS 1             | 1    | Connected to SPI1   |
| SENSOR | GPS                        | 0    | Connected to UART3  |
| SENSOR | Flash CS 1                 | 1    | Connected to SPI1   |
| SENSOR | SD Card CS 2               | 1    | Connected to SPI2   |
| SENSOR | Pressure Transducer (ADC)  | 0    | Connected to I2C1   |
| OUTPUT | MOSFET RCS Valve 1         | 1    | Digital      |
| OUTPUT | MOSFET RCS Valve 2         | 1    | Digital      |
| OUTPUT | Buzzer                     | 1    | Connected to PWM    |
| OUTPUT | LEDs                       | 3    | Connected to PWM    |

### CTR OBC
| TYPE   | ITEM                         | PINS | COMMENTS |
|--------|----------------------------|------|----------|
| COM    | UART1 TX (MAIN OBC)        | 1    |          |
| COM    | UART1 RX (MAIN OBC)        | 1    |          |
| OUTPUT | MOSFET RCS Valve 1         | 1    | Digital      |
| OUTPUT | MOSFET RCS Valve 2         | 1    | Digital      |
| OUTPUT | Buzzer                     | 1    | Connected to PWM    |
| OUTPUT | LEDs                       | 3    | Connected to PWM    |

### Summary
| MCU      | UART | I2C | SPI | PWM  | Digital | Total Pins |
|----------|------|-----|-----|------|---------|------------|
| MAIN OBC | 6    | 4   |  10 | 4    | 2       | 26         |
| CTR OBC  | 2    | 0   | 0   | 4    | 2       | 8          |

## Electronics Safety Equipment
A system with three types of switches is proposed for the electronic structure of the CUBESAT for reasons of safety, versatility, and with the aim of preventing discharges or long system operating times. However, some guidelines should be taken into account.

## Electrical Schematic Diagram 
The diagram is in this same folder in JPG format.


![Electrical Schematic Diagram](./Electrical_Diagram.jpg)

### Meaning of each abbreviation
| Abbreviation | Meaning | 
| :--- | :--- | 
| BAT1 | Battery Pack 1 | 
| BAT2 | Battery Pack 1 | 
| BAT3 | Battery Pack 2 | 
| BAT4 | Battery Pack 2 | 
| UED1 | Unknown Energy Distributor 1 | 
| UED2 | Unknown Energy Distributor 2 | 
| MS1 | Master Switch 1 | 
| MS2 | Master Switch 2 | 
| RBF1 | Remove Before Fly 1 | 
| RBF2 | Remove Before Fly 2 | 
| KS | Kill Switch | 
| BOOST | Booster | 
| R1 | Regulator 1 | 
| R2 | Regulator 2 | 
| CAM | Camera | 
| PT | Pressure Transducer |
| RW(1,2) | Reaction Wheels |
| RCS(1,2) | Electroválvulas |
| LS1 | Level Shifter 1 | 
| LS2 | Level Shifter 2 | 
| MF(1-8) | Mosfets | 
| B(1-14) | Borneras |

### Master Switch
This switch is used as redundancy for the rbf switch. If it were not there, the system's power supply would depend on it (RBF), which is very volatile and would pose many risks. It (MASTER) is used for security functions.

### RBF Switch
This switch consists of a removable pin that is inserted and removed through a hole in the fuselage when the CUBESAT is assembled inside the launch vehicle. The pin is inserted, the master switch is turned on, and then the pin is removed from the RBF to energize the computing and sensor area.

### Kill Switch
These switches are located in the actuator area and allow power to pass only when the CUBESAT is ejected from the launch vehicle.

### MOSFETS
These are electronic switches located in each actuator, there are two MOSFETs per actuator, both connected in series, for a total of eight. Each “column” has four, and each will be controlled by a different OBC: the main OBC will manage the power supply, and the control OBC will manage the actuators' activation.

## Pin Budget 
### 1. MAIN OBC: ESP32-WROOM-32

| Categoría | Ítem | Pin (GPIO) |
| :--- | :--- | :--- |
| COM | UART1 TX (CTR OBC) | 25 | 
| COM | UART1 RX (CTR OBC) | 34 | 
| COM | UART2 TX (CAM) | 26 | 
| COM | UART2 RX (CAM) | 35 |
| COM | UART3 TX (GPS) | 32 | 
| COM | UART3 RX (GPS) | 36 |
| I2C1 | SDA (IMU Fused/ADC/PCF8574) | 21 | 
| I2C1 | SCL (IMU Fused/ADC/PCF8574) | 22 | 
| SPI1 | SCK (Flash/ALT) | 18 | 
| SPI1 | MOSI (Flash/ALT) | 23 | 
| SPI1 | MISO (Flash/ALT) | 19 |
| SPI2 | SCK (IMU Raw/SD) | 16 | 
| SPI2 | MOSI (IMU Raw/SD) | 17 | 
| SPI2 | MISO (IMU Raw/SD) | 27 |
| OUTPUT | MOSFET 1 (RCS1) | 33 | 
| OUTPUT | MOSFET 2 (RCS2) | 4 | 

### 1.1 PCF8574
| Categoría | Ítem | Pin |
| :--- | :--- | :--- |
| COM | SDA (MAIN OBC) | SDA | 
| COM | SCL (MAIN OBC) | SCL | 
| CS1 | CS (Flash) | P0 | 
| CS1 | CS (ALT) | P1 | 
| CS2 | CS (IMU Raw) | P2 | 
| CS2 | CS (SD) | P3 | 
| OUTPUT | Buzzer | P4 | 
| OUTPUT | LEDs x3 | P5, P6, P7 | 

*Notes*
- PCF8574 is a pin expansion module that uses I2C communication.
- Pins 34, 35, and 36 are input-only.

### 2. CTR OBC: TEENSY 4.1

| Categoría | Ítem | Pin |
| :--- | :--- | :--- |
| COM | UART1 TX (MAIN OBC) | 1 | 
| COM | UART1 RX (MAIN OBC) | 0 | 
| OUTPUT | Buzzer | 6 | 
| OUTPUT | LEDs x3 | 9, 10, 11 | 
| OUTPUT | MOSFET 3 (RCS1) | 2 | 
| OUTPUT | MOSFET 4 (RCS2) | 3 | 

## FDIR Proposal
In the ELH system, we have identified a possible use of FDIR based on the mission's CONOPS during the descent phase, while the control algorithms necessary for the cubesat's roll are being executed. In this case, a hypothetical scenario imagined by the team that could affect the mission would be an opening of the solenoid valves without having been commanded by the OBC CONTROL. This opening could be measured through the use of pressure transducers, detecting a pressure drop not in accordance with the forecast. In this case, a possible use of FDIR would be to cut off the electrical supply to the solenoid valves, so that they cannot continue operating, ensuring an important mission safety factor.

## Power Budget
### CTR OBC
| ITEM                            | Operating Voltage (V) | Operating Consumption (mA) | Total Power Consumption (W) | Expected Operating Time (s) | Time Justification                          | Total Energy (Wh) |
| ------------------------------- | --------------------: | -------------------------: | --------------------------: | --------------------------: | ------------------------------------------- | --------------------: |
| MCU       |                 3.3 |                         80 |                         0.26 |                    720 | Control active only post-ejection |                      0.052 |
| Power Control CH1 (RCS Valve 1) |                  12.0 |                        640 |                         7.7 |                    300 | Intermittent roll corrections (~10% duty)   |          0.064 |
| Power Control CH2 (RCS Valve 2) |                  12.0 |                        640 |                         7.7 |                    300 | Intermittent roll corrections (~10% duty)   |          0.064 |
| Power Control (CAM)             |                   5.0 |                       1850 |                       9.25 |                    720 | Always ON post-ejection          |              1.85 |
| Buzzer                          |                   5.0 |                         50 |                         0.25 |                     60 | Distributed status beeps                    |           0.004 |
| LEDs (3×)                       |                   3.3 |                         60 |                         0.20 |                    720 | Continuous status indication                |           0.04 |

*NOTE:* Operating time based on the launch vehicle's descent rate approximation.


### Main OBC
| ITEM                 | Operating Voltage (V) | Operating Consumption (mA) | Total Power Consumption (W) | Expected Operating Time (s) | Time Justification                | Total Energy (Wh) |
| -------------------- | --------------------: | -------------------------: | --------------------------: | --------------------------: | --------------------------------- | --------------------: |
| MCU                  |                   3.3 |                        180 |                         0.60 |                    3600 | Always ON                         |           0.60 |
| IMU 9DOF Fused       |                   3.3 |                         30 |                         0.10 |                     720 | Only useful post-ejection         |           0.02 |
| IMU 9DOF Raw         |                   3.3 |                         15 |                         0.05 |                    1800 | Launch detection + backup         |          0.025 |
| Altimeter            |                   3.3 |                          5 |                        0.017 |                    1800 | Launch, apogee, descent awareness |         0.0085 |
| GPS                  |                   3.3 |                         45 |                         0.15 |                     720 | Post-ejection navigation          |           0.03 |
| Flash                |                   3.3 |                         30 |                         0.10 |                    3600 | Primary flight data logger        |           0.10 |
| SD Card (active)     |                   3.3 |                        100 |                         0.33 |                      40 | Landing-only data dump            |         0.0037 |
| Buzzer               |                   5.0 |                         50 |                         0.25 |                      60 | Distributed status beeps          |          0.004 |
| LEDs (3×)            |                   3.3 |                         60 |                         0.20 |                    3600 | Continuous status indication      |           0.20 |
| Pressure transducer  |                   5.0 |                         10 |                         0.05 |                    720 | Control active only post-eject     |           0.01 |
| ADC (P. Transducer)  |                   5.0 |                         10 |                         0.05 |                    720 | Control active only post-ejection  |           0.01 |
| PCF8574              |                   3.3 |                        0.01 |   0.000033 |                    720 | Control active only post-ejection  |      0,0000066 |

*NOTE:* Operating time based on the launch vehicle mission states to be taken into account. Subject to adjustments.

### Battery Capacity
| CATEGORY  | INCLUDED ELEMENTS | Total Current (mA) | Total Capacity (mAh) | COMMENTS | Proposed Battery | 
| :-------------------------: | ---------------------------------- | --------------------: | ----------------------------: | ----- | ----- |
| Battery Pack 1 | RCS valves (×2), Camera channel | 3130 | 1565 | For a half-hour operation | https://bigtronica.com/baterias-cargadores/baterias-li-ion/bateria-litio-ion-18650-2500mah-industrial-5053212018504.html |
| Battery Pack 2 | All MCUs (CTR, MAIN), IMUs, GPS, Altimeter, Flash, SD, Pressure transducer, ADC, Leds, Buzzers |  725  | 1088 | For a one-and-a-half-hour operation | https://www.mactronica.com.co/bateria-litio-1000mah-37v-18650?srsltid=AfmBOooXohHs6zvhAGscgko1IeH-YPE4X5k91hoNfmuCgwN3wX7bfoiP |

### Summary
| CATEGORY  | INCLUDED ELEMENTS | Total Energy (Wh) |
| :-------------------------: | ---------------------------------- | --------------------: | 
| Actuation & Camera | RCS valves (×2), Camera channel                   |         ≈ 2 Wh | 
| Computation & Sensors   | All MCUs (CTR,MAIN), IMUs, GPS, Altimeter, Flash, SD, Pressure transducer, ADC, LEDs, Buzzers |         ≈ 1,12 Wh | 

The total energy budget is divided into two independent battery domains with margins applied according to the dominant risks of each domain. The first domain, corresponding to actuation and camera (reaction wheels, RCS valves, camera channel), is active only during the mission phase after CubeSat ejection and is therefore dominated by short-duration, high-power operation. For this domain, a safety margin of at least 50% of total energy consumption is used, this result in a required battery capacity of approximately **3.1 Wh** for the actuation and camera battery.

The second domain supplies all computation and sensing functions, including all MCUs, sensors, memory devices, LEDs, and buzzers, and is dominated by long-duration low-power operation since the PAD phase. For this domain, the base energy consists of approximately 1,12 Wh consumed during a one-hour full mission period, with an additional small allowance for EPS quiescent currents and leakage. After applying the safety margin, the resulting required battery capacity for the computation and sensor domain is approximately **1.85 Wh**.


