NOTE: It is missing to add a pressure sensor from the RCS system

# ELH-ELS (Electronics Hardware and Software) Architecture

## Description
The system is based on two electrically identical battery packs providing full energy redundancy, with a dedicated Power Management Controller (PMC) acting as the sole authority for power supervision and distribution, where one pack at a time is selected per domain while the other remains isolated as a hot spare; the PMC continuously monitors voltage, current and faults, actively selects which pack powers the actuator bus, and individually enables, disables, or isolates each actuator channel through electronic switches, while the power rail feeding the onboard computers is protected by independent hardware supervisors (undervoltage, overcurrent and brown-out) that guarantee deterministic behavior even in the presence of PMC firmware failures, with the PMC only monitoring this rail; above this power layer operate two onboard computers with strictly separated roles, where the MAIN OBC performs all system-level functions including sensor acquisition (GPS, fused IMU, raw IMU and barometer), state estimation, data logging to flash and SD, communications, mode management, fault handling and PMC supervision, and distributes validated state information, while the CTR OBC is dedicated exclusively to real-time control and flight dynamics, generating actuator commands based on the received state without handling power, logging or communications, resulting in a modular, fault-tolerant and safety-oriented architecture that cleanly separates electrical authority, system intelligence and control execution.

## Pin and Interface Budget
### PMC
| TYPE   | ITEM                              | PINS | COMMENTS |
|--------|-----------------------------------|------|----------|
| OUTPUT | Pack selector                     | 1    | For battery selection |
| OUTPUT | Power Control CH1 (RW motor 1)     | 1    | Requirement by RW|
| OUTPUT | Power Control CH2 (RW motor 2)     | 1    | Requirement by RW|
| OUTPUT | Power Control CH3 (RCS Valve 1)    | 1    |Requirement by RCS|
| OUTPUT | Power Control CH4 (RCS Valve 2)    | 1    |Requirement by RCS|
| OUTPUT | Power Control CH5 (CAM)           | 1    |Requirement by Camera Control|
| OUTPUT | Power Control CH6 (SOL)           | 1    |Requirement by Solar Panel Array|
| COM    | UART1 TX (MAIN OBC)               | 1    | For communication with main OBC|
| COM    | UART1 RX (MAIN OBC)               | 1    |For communication with main OBC |
| COM    | SDA I2C1                          | 1    |I2C1 bus definition |
| COM    | SCL I2C1                          | 1    |I2C1 bus definition |
| SENSOR | CH1 Power Monitor IC              | 0    | Connected to I2C1     |
| SENSOR | CH2 Power Monitor IC              | 0    | Connected to I2C1     |
| SENSOR | CH3 Power Monitor IC              | 0    | Connected to I2C1     |
| SENSOR | CH4 Power Monitor IC              | 0    | Connected to I2C1     |
| SENSOR | CH5 Power Monitor IC              | 0    | Connected to I2C1     |
| SENSOR | CH6 Power Monitor IC              | 0    | Connected to I2C1     |
| SENSOR | Pack 1 Power Monitor IC           | 0    | Connected to I2C1     |
| SENSOR | Pack 2 Power Monitor IC           | 0    | Connected to I2C1     |
| OUTPUT | Buzzer                            | 1    | PWM      |
| OUTPUT | LEDs                              | 3    | PWM      |

### MAIN OBC
| TYPE   | ITEM                       | PINS | COMMENTS |
|--------|----------------------------|------|----------|
| COM    | UART1 TX (PMC)              | 1    | For communication with PMC |
| COM    | UART1 RX (PMC)              | 1    | For communication with PMC |
| COM    | UART2 TX (CTR OBC)          | 1    | For communication with Control OBC |
| COM    | UART2 RX (CTR OBC)          | 1    | For communication with Control OBC |
| COM    | UART3 TX (CAM)              | 1    | For communication with Cameras Module |
| COM    | UART3 RX (CAM)              | 1    | For communication with Cameras Module |
| COM    | UART4 TX (GPS)              | 1    | For GPS module connection |
| COM    | UART4 RX (GPS)              | 1    | For GPS module connection |
| COM    | UART5 TX (BT)              | 1    | For BT module coneection |
| COM    | UART5 RX (BT)              | 1    | For BT module coneection |
| COM    | SDA I2C1                    | 1    | I2C1 bus definition |
| COM    | SCL I2C1                    | 1    | I2C1 bus definition |
| COM    | SDA I2C2                    | 1    | I2C2 bus definition |
| COM    | SCL I2C2                    | 1    | I2C2 bus definition |
| COM    | SCK SPI1                    | 1    | SPI1 bus definition|
| COM    | MOSI SPI1                   | 1    | SPI1 bus definition |
| COM    | MISO SPI1                   | 1    | SPI1 bus definition |
| COM    | SCK SPI2                    | 1    |SPI2 bus definition |
| COM    | MOSI SPI2                   | 1    |SPI2 bus definition |
| COM    | MISO SPI2                   | 1    | SPI2 bus definition|
| SENSOR | IMU 9DOF Fused              | 0    | Connected to I2C1     |
| SENSOR | IMU 9DOF Raw                | 0    | Connected to I2C2 |
| SENSOR | Altimeter                  | 0    | Connected to I2C1 |
| SENSOR | GPS                        | 0    | Connected to UART5    |
| SENSOR | Flash                      | 1    | Connected to SPI1     |
| SENSOR | SD Card                    | 1    | Connected to SPI2     |
| COM    | Bluetooth Capability       | 0    | Connected to UART     |
| OUTPUT | Buzzer                     | 1    | Connected to PWM      |
| OUTPUT | LEDs                       | 3    | Connected to PWM      |
| OUTPUT | SOL Control Channel        | 1    |          |

### CTR OBC
| TYPE   | ITEM                         | PINS | COMMENTS |
|--------|------------------------------|------|----------|
| COM    | UART1 TX (MAIN OBC)           | 1    |          |
| COM    | UART1 RX (MAIN OBC)           | 1    |          |
| OUTPUT | Control CH1 (RW motor 1)      | 1    | PWM      |
| OUTPUT | Control CH2 (RW motor 2)      | 1    | PWM      |
| OUTPUT | Control CH3 (RCS Valve 1)     | 1    | PWM      |
| OUTPUT | Control CH4 (RCS Valve 2)     | 1    | PWM      |
| OUTPUT | Control CH5 (RCS Valve 3)     | 1    | PWM      |
| OUTPUT | Control CH6 (RCS Valve 4)     | 1    | PWM      |
| OUTPUT | Buzzer                       | 1    | PWM      |
| OUTPUT | LEDs                         | 3    | PWM      |

### Summary
| MCU      | UART | I2C | SPI | PWM  | Total Pins |
|----------|------|-----|-----|------|------------|
| PMC      | 1    | 1   | 0   | 4    | 15         |
| MAIN OBC | 5    | 2   | 2   | 4    | 27         |
| CTR OBC  | 1    | 0   | 0   | 10   | 12         | 
---
*NOTE 1:* Main OBC shall include bluethoot capabilities \
*NOTE 2:* Control OBC shall be a control type MCU \
*NOTE 3:* Main OBC can use RTOS


## Power Budget
### PMC
| ITEM                            | Operating Voltage (V) | Operating Consumption (mA) | Total Power Consumption (W) | Expected Operating Time (s) | Time Justification                          | Total Energy (Wh) |
| ------------------------------- | --------------------: | -------------------------: | --------------------------: | --------------------------: | ------------------------------------------- | --------------------: |
| MCU                             |                   3.3 |                         18 |                       0.060 |                     250 | Always ON (PAD, ascent, mission)            |           0.00417 |
| Pack selector                   |                   5.0 |                         20 |                        0.10 |                     250 | Continuous EPS supervision                  |           0.00694 |
| Power Control CH1 (RW motor 1)  |                  12.0 |                        800 |                         9.6 |                     223 | Continuous roll stabilization post-ejection |             0.595 |
| Power Control CH2 (RW motor 2)  |                  12.0 |                        800 |                         9.6 |                     223 | Continuous roll stabilization post-ejection |             0.595 |
| Power Control CH3 (RCS Valve 1) |                  12.0 |                        640 |                         7.7 |                      22 | Intermittent roll corrections (~10% duty)   |            0.0470 |
| Power Control CH4 (RCS Valve 2) |                  12.0 |                        640 |                         7.7 |                      22 | Intermittent roll corrections (~10% duty)   |            0.0470 |
| Power Control CH5 (CAM)         |                   5.0 |                       1850 |                        9.25 |                     250 | Explicit requirement: full mission          |             0.642 |
| Power Control CH6 (SOL)         |                   5.0 |                        200 |                         1.0 |                       5 | Single-shot nichrome heating                |           0.00139 |
| CH1 Power Monitor IC            |                   3.3 |                          5 |                       0.017 |                     250 | Continuous power monitoring                 |           0.00118 |
| CH2 Power Monitor IC            |                   3.3 |                          5 |                       0.017 |                     250 | Continuous power monitoring                 |           0.00118 |
| CH3 Power Monitor IC            |                   3.3 |                          5 |                       0.017 |                     250 | Continuous power monitoring                 |           0.00118 |
| CH4 Power Monitor IC            |                   3.3 |                          5 |                       0.017 |                     250 | Continuous power monitoring                 |           0.00118 |
| CH5 Power Monitor IC            |                   3.3 |                          5 |                       0.017 |                     250 | Continuous power monitoring                 |           0.00118 |
| CH6 Power Monitor IC            |                   3.3 |                          5 |                       0.017 |                     250 | Continuous power monitoring                 |           0.00118 |
| Pack 1 Power Monitor IC         |                   3.3 |                          5 |                       0.017 |                     250 | Continuous pack monitoring                  |           0.00118 |
| Pack 2 Power Monitor IC         |                   3.3 |                          5 |                       0.017 |                     250 | Continuous pack monitoring                  |           0.00118 |
| Buzzer                          |                   5.0 |                         50 |                        0.25 |                      20 | Distributed status beeps                    |           0.00139 |
| LEDs (3×)                       |                   3.3 |                         60 |                        0.20 |                     250 | Continuous status indication                |            0.0139 |


### Main OBC
| ITEM                 | Operating Voltage (V) | Operating Consumption (mA) | Total Power Consumption (W) | Expected Operating Time (s) | Time Justification                | Total Energy (Wh) |
| -------------------- | --------------------: | -------------------------: | --------------------------: | --------------------------: | --------------------------------- | --------------------: |
| MCU                  |                   3.3 |                        180 |                        0.60 |                     250 | Always ON                         |            0.0417 |
| IMU 9DOF Fused       |                   3.3 |                         30 |                        0.10 |                     223 | Only useful post-ejection         |           0.00619 |
| IMU 9DOF Raw         |                   3.3 |                         15 |                        0.05 |                     250 | Launch detection + backup         |           0.00347 |
| Altimeter            |                   3.3 |                          5 |                       0.017 |                     250 | Launch, apogee, descent awareness |           0.00118 |
| GPS                  |                   3.3 |                         45 |                        0.15 |                     223 | Post-ejection navigation          |           0.00929 |
| Flash                |                   3.3 |                         30 |                        0.10 |                     250 | Primary flight data logger        |           0.00694 |
| SD Card (active)     |                   3.3 |                        100 |                        0.33 |                      10 | Landing-only data dump            |           0.00092 |
| Bluetooth Capability |                   3.3 |                         40 |                        0.13 |                       0 | Disabled in flight                |           0.00000 |
| Buzzer               |                   5.0 |                         50 |                        0.25 |                      20 | Distributed status beeps          |           0.00139 |
| LEDs (3×)            |                   3.3 |                         60 |                        0.20 |                     250 | Continuous status indication      |            0.0139 |


### CTR OBC
| ITEM      | Operating Voltage (V) | Operating Consumption (mA) | Total Power Consumption (W) | Expected Operating Time (s) | Time Justification                | Total Energy (Wh) |
| --------- | --------------------: | -------------------------: | --------------------------: | --------------------------: | --------------------------------- | --------------------: |
| MCU       |                   3.3 |                         80 |                        0.26 |                     223 | Control active only post-ejection |            0.0161 |
| Buzzer    |                   5.0 |                         50 |                        0.25 |                      20 | Distributed status beeps          |           0.00139 |
| LEDs (3×) |                   3.3 |                         60 |                        0.20 |                     223 | Active during CubeSat mission     |            0.0124 |

### Raw Summary
| CATEGORY                         | INCLUDED ELEMENTS                                                                                                   | Total Energy (Wh) | COMMENT                                                      |
| -------------------------------- | ------------------------------------------------------------------------------------------------------------------- | --------------------: | ------------------------------------------------------------ |
| Actuation & Payload Channels | RW motors (×2), RCS valves (×2), Camera channel, SOL nichrome channel                                               |         ≈ 1.93 Wh | Dominant energy driver; camera + RW are primary contributors |
| Computation, Sensors & ICs   | All MCUs (PMC, Main OBC, CTR OBC), IMUs, GPS, Altimeter, Flash, SD , Power Monitor ICs, LEDs, Buzzers |         ≈ 0.15 Wh | Control, sensing, monitoring, and housekeeping               |

### Final decision
The total energy budget is divided into two independent battery domains with margins applied according to the dominant risks of each domain. The first domain, corresponding to actuation and payload channels (reaction wheels, RCS valves, camera channel, and the single-shot nichrome deployment channel), is active only during the mission phase after CubeSat ejection and is therefore dominated by short-duration, high-power operation. For this domain, the raw mission energy of approximately 1.93 Wh is increased by an allowance for EPS conduction and switching losses (≈10%), followed by a DC/DC conversion and wiring efficiency margin of 15%, and a system-level design margin of 30% to cover uncertainty in duty cycles and actuator usage. These combined margins result in a required battery capacity of approximately 3.0 Wh for the actuation and payload battery.

The second domain supplies all computation, sensing, and housekeeping functions, including all MCUs, sensors, memory devices, power monitor ICs, LEDs, and buzzers, and is dominated by long-duration low-power operation during the PAD phase. For this domain, the base energy consists of approximately 0.15 Wh during the mission plus about 1.5 Wh consumed during a three-hour PAD period, with an additional small allowance for EPS quiescent currents and leakage. Margins applied to this domain include a 15% margin for DC/DC conversion and distribution losses, a 20% derating to account for temperature effects and battery aging, and a 30% design margin to cover variability in PAD duration and operational modes. After applying these margins, the resulting required battery capacity for the computation and sensor domain is approximately 3.2 Wh.