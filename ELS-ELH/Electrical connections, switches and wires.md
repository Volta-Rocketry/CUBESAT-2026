# Electrical Connections, Switches, and Wires

## Purpose of this Document
This document defines part of the proposed final structure system for the CUBESAT for 2026 IREC competition, focusing on electronic safety equipment.

## Electronics Safety Equipment
A system with three types of switches is proposed for the electronic structure of the CUBESAT for reasons of safety, versatility, and with the aim of preventing discharges or long system operating times. However, some guidelines should be taken into account.

## Electrical Schematic Diagram 
The diagram is in this same folder in JPG format.


![Electrical Schematic Diagram ](./Electrical_Diagram.jpg)

### Master Switch
This switch is used as redundancy for the rbf switch. If it were not there, the system's power supply would depend on it (RBF), which is very volatile and would pose many risks. It (MASTER) is used for security functions.

### RBF Switch
This switch consists of a removable pin that is inserted and removed through a hole in the fuselage when the CUBESAT is assembled inside the launch vehicle. The pin is inserted, the master switch is turned on, and then the pin is removed from the RBF to energize the computing and sensor area.

### Kill Switch
These switches are located in the actuator area and allow power to pass only when the CUBESAT is ejected from the launch vehicle.

### MOSFETS
These are electronic switches located in each actuator, there are two MOSFETs per actuator, both connected in series, for a total of eight. Each “column” has four, and each will be controlled by a different OBC: the main OBC will manage the power supply, and the control OBC will manage the actuators' activation.

# Configuración de Hardware - Sistema de Cohetería Experimental 2026

Este documento detalla el "Pin Budget" final para los dos ordenadores de a bordo (OBC). Se ha implementado una arquitectura de **Seguridad en Serie (Armado y Disparo)** para todos los actuadores de 12V.

---

## Pin Budget (Preliminar)
### 1. MAIN OBC: ESP32-S3 (DevKitC-1)

| Categoría | Ítem | Pin (GPIO) |
| :--- | :--- | :--- |
| COM | UART1 TX (CTR OBC) | 43 | 
| COM | UART1 RX (CTR OBC) | 44 | 
| COM | UART2 TX (CAM) | 15 | 
| COM | UART2 RX (CAM) | 16 |
| COM | UART3 TX (GPS) | 17 | 
| COM | UART3 RX (GPS) | 18 |
| I2C1 | SDA (IMU Fused) | 4 | 
| I2C1 | SCL (IMU Fused) | 5 | 
| I2C2 | SDA (ADC) | 6 | 
| I2C2 | SCL (ADC) | 7 | 
| SPI1 | SCK (Flash/ALT) | 12 | 
| SPI1 | MOSI (Flash/ALT) | 11 | 
| SPI1 | MISO (Flash/ALT) | 13 |
| CS1 | CS (Flash) | 10 | 
| CS1 | CS (ALT) | 14 | 
| SPI2 | SCK (IMU Raw/SD) | 36 | 
| SPI2 | MOSI (IMU Raw/SD) | 35 | 
| SPI2 | MISO (IMU Raw/SD) | 37 |
| CS2 | CS (IMU Raw) | 47 | 
| CS2 | CS (SD) | 48 | 
| OUTPUT | Buzzer | 2 | 
| OUTPUT | LEDs x3 | 38, 39, 40 | 
| OUTPUT | MOSFET 1 (RW1) | 21 | 
| OUTPUT | MOSFET 2 (RW2) | 41 | 
| OUTPUT | MOSFET 3 (RCS1) | 42 | 
| OUTPUT | MOSFET 4 (RCS2) | 9 | 

### 2. CTR OBC: TEENSY 4.1

| Categoría | Ítem | Pin |
| :--- | :--- | :--- |
| COM | UART1 TX (MAIN OBC) | 1 | 
| COM | UART1 RX (MAIN OBC) | 0 | 
| OUTPUT | Buzzer | 6 | 
| OUTPUT | LEDs x3 | 9, 10, 11 | 
| OUTPUT | MOSFET 5 (RW1) | 2 | 
| OUTPUT | MOSFET 6 (RW2) | 3 | 
| OUTPUT | MOSFET 7 (RCS1) | 4 | 
| OUTPUT | MOSFET 8 (RCS2) | 5 | 





