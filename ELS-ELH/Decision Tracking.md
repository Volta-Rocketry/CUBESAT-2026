# ELH-ELS (Electronics Hardware and Software) Decision Tracking

## Objective and Scope
The purpose of this document is to formally capture, justify, and maintain traceability of all major technical and architectural decisions related to the ELH-ELS (Electronics Hardware and Software) system.

The scope includes, but is not limited to:

- Hardware architecture and component selection
- Firmware and software architecture
- Interfaces, protocols, and data flows
- Safety, redundancy, and fault-handling strategies
- Power, performance, and reliability trade-offs

### Note
All prices are quoted in USD and, unless otherwise specified, electronic component pricing is sourced from the Mouser Electronics website.

## Decisions
### Architecture
Based on team reviews, CubeSat standards, and system-level requirements, Architecture #1 (PMC with dual OBC) has been selected as the baseline.

This architecture provides the highest level of reliability, fault tolerance, and compliance with mission requirements. The use of a Power Management Controller (PMC) combined with dual On-Board Computers (OBCs) enables a modular, redundant, and safety-oriented design, ensuring:

- Robust fault detection, isolation, and recovery (FDIR)
- Controlled power distribution and system-level supervision
- Redundant command and data handling
- Graceful degradation in case of subsystem failure

### Component Selection - Power
#### Load Switches
Three architectures were evaluated for controlling and protecting the CubeSat power distribution rails: (1) discrete MOSFET switches driven directly by the MCU, (2) integrated eFuse / hot-swap power switches, and (3) mechanical relays. A discrete MOSFET approach provides basic on/off capability but requires additional circuitry for inrush control, short-circuit protection, reverse-current blocking, and thermal protection, making it highly layout-dependent and sensitive to EMI and PCB parasitics. Mechanical relays, while simple to control, are unsuitable due to vibration sensitivity, arcing, slow response time, and limited lifetime. Integrated eFuses combine a high-side MOSFET with programmable current limiting, soft-start, short-circuit and thermal protection, reverse-current blocking, and fault reporting in a single device, requiring only minimal external components and a simple MCU interface. This architecture provides the highest level of electrical protection with the lowest implementation complexity and risk, making the eFuse-based solution the most robust and appropriate choice for the CubeSat Power Management Computer.

The devices listed in the following table represent the full spectrum of integrated eFuse solutions available for protected power distribution, spanning from low-power to very high-current applications. These parts were selected for comparison because they share the same architectural model, integrated back-to-back MOSFETs, programmable current limiting, inrush control, reverse-current blocking, thermal protection, and MCU-friendly EN/FLT/PGOOD interfaces, while differing mainly in voltage range, current capability, and $R_{DS_{on}}$. 

Lower-current parts such as the TPS25940 and TPS25946 are well suited for CubeSat subsystem rails, offering sufficient current capacity with lower cost and simpler PCB thermal requirements. Higher-current devices (TPS25983 and TPS25984x) support much larger loads but require larger packages, heavier copper, wider pours, and more thermal management, significantly increasing PCB complexity, mass, and layout risk. 

| Part                                 | Voltage  | Current  | Good For                                  | Price (1/10/25)|
| ------------------------------------ | -------- | -------- | ----------------------------------------- |----|
| **TPS25940**                         | 2.7–18 V | ~0.6–5 A | Base solution for low-power CubeSat rails | 2.75/2.02/1.85|
| **TPS25946**                         | 2.7–23 V | ~0.5–6 A | Higher voltage & lower R<sub>ds(on)</sub> | 1.43/1.04/0.95|
| **TPS25983**                         | 2.7–26 V | ~2–20 A  | Mid-high power rails                      |3.75/2.84/2.61|
| **TPS25984x**                        | 4.5–16 V | ~10–55 A | Heavy power distribution                  |8.21/5.74/5.47|

**NOTE: DECISION FROM THIS OPTION WILL TAKE PLACE AFTER THE POWER REQUIREMNTS PER CHANEL HAVE BEEN RECIEVED**

#### Power Monitor ICs
There are three viable approaches to measure power consumption in a system: (1) measuring voltage and current separately using analog dividers and shunt resistors connected to the MCU ADC, which requires external amplifiers, careful filtering, and calibration; (2) measuring only current with a shunt plus an external current-sense amplifier and then digitizing it with the ADC; or (3) using integrated high-side power monitor ICs such as the INA28x family, which include a precision amplifier, ADC, and digital interface in a single device. 

While the first two options are feasible, they introduce significant analog complexity, noise sensitivity, and calibration burden. In contrast, INA-class power monitors provide accurate high-side current, bus voltage, and power measurements with minimal external components, making them the most robust, scalable, and low-risk solution for multi-rail power telemetry and protection in a flight-critical or high-reliability power system.

The devices listed in the following table were selected for high-side current and voltage monitoring of the CubeSat power rails, balancing accuracy, bus voltage range, and integration features. The INA226 operates over 0–36 V with a 16-bit ADC and ~0.1% accuracy, making it a reliable industry-standard choice for general-purpose telemetry. The INA238 extends the bus voltage range to 0–85 V with improved accuracy (~0.05%) and a radiation-tolerant design, offering heritage suitable for aerospace applications. The MAX34417 supports 0–36 V, 16-bit resolution, and includes built-in alert functionality and filtering, simplifying system monitoring and fault detection. These devices were chosen over older alternatives such as the INA219 because they provide higher precision, true high-side sensing, alerting, and better long-term stability, which are critical for accurate measurement of multiple power rails in a CubeSat environment.


| Part         | Bus Voltage | ADC    | Accuracy | Addressing | Notes                       | Price (1/10/25)|
| ------------ | ----------- | ------ | -------- | ---------- | --------------------------- | --|
| **INA226**   | 0–36 V      | 16-bit | ~0.1%    | 16 addr    | Industry workhorse          | 3.81/2.88/2.65
| **INA238**   | 0–85 V      | 16-bit | ~0.05%   | 16 addr    | Radiation-tolerant heritage | 3.17/2.38/2.18
| **MAX34417** | 0–36 V      | 16-bit | ~0.1%    | 8 addr     | Built-in alert + filters    | 15.26/11.60/11.12

**NOTE: DECISION FROM THIS OPTION WILL TAKE PLACE AFTER THE POWER REQUIREMNTS PER CHANEL HAVE BEEN RECIEVED**

#### Battery pack switchover alternatives
For providing a single output from two battery packs, three approaches were considered: (1) simple diodes for OR-ing, (2) MCU-controlled MOSFET switches, and (3) ideal-diode OR-ing controllers with external MOSFETs. Simple diodes are electrically safe but introduce significant voltage drop and wasted energy, which reduces usable battery capacity and efficiency. MCU-controlled switches offer apparent flexibility but carry high risk of cross-conduction, brownouts, and failure in case of firmware errors, making them unsuitable for critical power paths. Ideal-diode OR-ing controllers combine automatic source selection, low forward voltage drop, and inherent reverse-current blocking using external MOSFETs, providing seamless switchover without firmware dependency and minimal losses. This architecture achieves the lowest practical complexity while maintaining safety and efficiency, making the ideal-diode OR-ing the most robust and appropriate solution for CubeSat battery pack management.

The ICs listed in the following table were selected as suitable solutions for automatic battery pack switchover based on voltage range, efficiency, and integration features. The LM74700D-Q1 supports a wide voltage range (~3.2–65 V) with very low forward voltage drop, making it ideal for robust, low-loss selection across a broad set of battery chemistries and voltages. The LTC4412 provides automotive-grade ideal-diode control for 2.5–28 V systems, offering proven reliability, low quiescent current, and seamless switchover, suitable for standard CubeSat Li-ion packs. The MAX5079 operates up to 13.2 V and includes built-in ORing logic and status detection, simplifying system monitoring and reducing external components. These devices were chosen over discrete MOSFET logic or passive diodes because they ensure safe, low-loss, and automatic selection without firmware dependency, while minimizing PCB complexity and design risk.

| IC              | Best for battery packs up to… | Key benefit                          | Price (1/10/25)|
| --------------- | ----------------------------- | ------------------------------------ | ---|
| **LM74700D-Q1** | ~3.2 – 65 V                   | Very low loss, wide voltage range    | 2.41/1.79/1.64
| **LTC4412**     | ~2.5 – 28 V                   | Automotive-class ideal diode control | 15.23/11.95/11.46
| **MAX5079**     | ~2.75 – 13.2 V                | Built-in detection and ORing logic   | 11.32/8.86/8.73

**NOTE: DECISION FROM THIS OPTION WILL TAKE PLACE AFTER THE POWER REQUIREMNTS PER CHANEL HAVE BEEN RECIEVED**

#### Kill switches

### Component Selection - OBCs
#### PMC - MCU 

#### Main OBC - MCU

#### Main OBC - 9DOF IMU Fused

#### Main OBC - 9DOF IMU Raw

#### Main OBC - Altimeter

#### Main OBC - GPS

#### Main OBC - Flash Module

#### Main OBC - Bluethoot Module