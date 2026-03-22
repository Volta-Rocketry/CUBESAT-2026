# Electrical Connections, Switches, and Wires

## Purpose of this Document
This document defines part of the proposed final structure system for the CUBESAT for 2026 IREC competition, focusing on electronic safety equipment and cable selection.

## Electronics Safety Equipment
A system with three types of switches is proposed for the electronic structure of the CUBESAT for reasons of safety, versatility, and with the aim of preventing discharges or long system operating times. However, some guidelines should be taken into account.

## Electrical Schematic Diagram 
```mermaid
flowchart LR
    BAT1[Battery 1]
    BAT2[Battery 2]
    BAT3[Battery 3]
    BAT4[Battery 4]
    B1[Bornera 1]
    B2[Bornera 2]
    B3[Bornera 3]
    B4[Bornera 4]
    B5[Bornera 5]
    B6[Bornera 6]
    B7[Bornera 7]
    B8[Bornera 8]
    B9[Bornera 9]
    B10[Bornera 10]
    B11[Bornera 11]
    B12[Bornera 12]
    B13[Bornera 13]
    MCU1[MAIN MCU]
    MCU2[CTR MCU]
    IMU1[IMU 9DOF Fused]
    IMU2[IMU 9DOF Raw]
    ALT[Altimeter]
    GPS[GPS]
    FLASH[Flash Module]
    SD[SD Card]
    PT[Pressure Transducer]
    CAM[Camera]
    BUZ1[Buzzer 1]
    BUZ2[Buzzer 2]
    LED1[LED 1]
    LED2[LED 2]
    CH1[RW motor 1]
    CH2[RW motor 2]
    CH3[RCS Valve 1]
    CH4[RCS Valve 2]
    ED1[Unknown Energy Distributor 1]
    ED2[Unknown Energy Distributor 2]
    MS1[Master Switch]
    MS2[Master Switch]
    KS[Kill Switch]
    RBF1[Remove Before Fly Switch]
    RBF2[Remove Before Fly Switch]
    R1[Regulador 1]
    R2[Regulador 2]
    MOS1[MOSFET 1]
    MOS2[MOSFET 2]
    MOS3[MOSFET 3]
    MOS4[MOSFET 4]
    BOOST[Booster]

    BAT1 --> B1
    BAT2 --> B1
    B1 --> ED1
    ED1 --> B2
    B2 --> MS1
    MS1 --> B3
    B3 --> RBF1
    RBF1 --> B4
    B4 --> KS
    KS --> B5
    B5 --> MOS1
    B5 --> MOS2
    B5 --> MOS3
    B5 --> MOS4
    B5 --> R1 
    MOS1 --> B6
    MOS2 --> B7
    MOS3 --> B8
    MOS4 --> B9
    B6 --> CH1
    B7 --> CH2
    B8 --> CH3
    B9 --> CH4
    R1 --> B10
    B10 --> CAM

    BAT3 --> B11
    BAT4 --> B11
    B11 --> ED2
    ED2 --> B12
    B12 --> MS2
    MS2 --> B13
    B13 --> RBF2
    RBF2 --> BOOST
    BOOST --> R2
    R2 --> IMU1
    R2 --> IMU2
    R2 --> ALT
    R2 --> GPS
    R2 --> FLASH
    BOOST --> SD
    BOOST --> PT
    BOOST --> MCU1
    BOOST --> MCU2
    MCU1 --> LED1
    MCU1 --> BUZ1
    MCU2 --> LED2
    MCU2 --> BUZ2

```

### Master Switch
This switch is used as redundancy for the rbf switch. If it were not there, the system's power supply would depend on it (RBF), which is very volatile and would pose many risks. It (MASTER) is used for security functions.

### RBF Switch
This switch consists of a removable pin that is inserted and removed through a hole in the fuselage when the CUBESAT is assembled inside the launch vehicle. The pin is inserted, the master switch is turned on, and then the pin is removed from the RBF to energize the computing and sensor area.

### Kill Switch
These switches are located in the actuator area and allow power to pass only when the CUBESAT is ejected from the launch vehicle.

### MOSFETS
These are electronic switches located in each actuator, allowing power to pass or not through signals sent by the MAIN OBC.


## Analysis and Selection of Cables

| ITEM      | Operating Voltage (V) | Total Power Consumption (W) | Operating Consumption (A) | Caliber of Cables (AWG) |
| ------------------------------- | --------------------: | -------------------------: | --------------------------: |--------------------------: |
| RW motors  |    12 |   9.6 |    0.8 | 24-26 |
| Valves     |    12 |   7.7 |    0.6 | 26-28 |
| Camera     |     5 |  9.25 |   1.85 | 22-24 |
| Buzzers    |     5 |  0.25 |   0.05 | 30    |
| Leds       |   3.3 |   0.2 |   0.06 | 30    |
| Tierra     |       |       |        | 22    |


## 1. MAIN OBC: ESP32-S3 DevKit C1

| Categoría | Ítem | Pin (GPIO) |
| :--- | :--- | :--- | 
| **COM** | UART TX (CTR OBC) | **GPIO 43** | 
| **COM** | UART RX (CTR OBC) | **GPIO 44** | 
| **COM** | UART3 TX (GPS) | **GPIO 17** | 
| **COM** | UART3 RX (GPS) | **GPIO 18** | 
| **I2C1** | SDA (Fused Data) | **GPIO 4** | 
| **I2C1** | SCL (Fused Data) | **GPIO 5** | 
| **SPI1** | SCK (Flash/SD) | **GPIO 12** | 
| **SPI1** | MOSI (Flash/SD) | **GPIO 11** | 
| **SPI1** | MISO (Flash/SD) | **GPIO 13** | 
| **CS** | CS Flash | **GPIO 10** | 
| **CS** | CS Micro SD | **GPIO 14** | 
| **SPI2** | SCK (Sensores) | **GPIO 36** | 
| **SPI2** | MOSI (Sensores) | **GPIO 35** | 
| **SPI2** | MISO (Sensores) | **GPIO 37** | 
| **CS** | CS IMU Raw | **GPIO 47** | 
| **CS** | CS Altimeter | **GPIO 48** | 
| **I2C2**| SDA (ADC) | **GPIO 6** | 
| **I2C2**| SCL (ADC) | **GPIO 7** | 
| **COM** | UART2 TX (CAM) | **GPIO 15** | 
| **COM** | UART2 RX (CAM) | **GPIO 16** | 
| **OUTPUT**| Buzzer 1 | **GPIO 2** |
| **OUTPUT**| LEDs | **38, 39, 40**| 

## 2. CTR OBC: TEENSY 4.1

| Categoría | Ítem | Pin |
| :--- | :--- | :--- | 
| **COM** | UART1 TX (MAIN) | **Pin 1** | 
| **COM** | UART1 RX (MAIN) | **Pin 0** | 
| **OUTPUT**| Buzzer 2 | **6** | 
| **OUTPUT**| LEDs | **9, 10, 11** | 