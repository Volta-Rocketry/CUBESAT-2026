# ELH-ELS (Electronics Hardware and Software) Architecture Proposal

## Battery Packs Concept
In all proposed architectures, the term battery packs refers to two electrically identical sets of batteries, equal in chemistry, capacity, nominal voltage, and performance envelope. These packs are designed to be interchangeable from an electrical standpoint. However, only one pack is actively used at any given time, while the other remains available as a redundant or alternate source.

## General Elements
Across all proposed architectures, a common set of core subsystems is preserved to ensure functional consistency, comparability, and predictable system behavior. These elements are treated as architecture-invariant, meaning their role, and fundamental interaction model remain unchanged regardless of how processing, control, or power management responsibilities are distributed.

The system includes a GPS receiver used for absolute positioning, velocity estimation, and time reference. This provides global navigation data that complements inertial and barometric measurements and serves as a long-term correction source for state estimation algorithms.

Two inertial measurement units are incorporated with distinct roles. The first is an IMU with integrated sensor fusion capabilities, intended to provide a reliable, low-latency estimate of attitude and angular rates with minimal computational overhead. This fused output is primarily used for fast-loop control, initialization, and redundancy. In parallel, a second IMU dedicated exclusively to raw sensor data acquisition is included. This device provides unprocessed accelerometer, gyroscope and magnetometer measurements, enabling custom filtering, advanced state estimation, post-flight analysis, and algorithm validation independent of vendor-provided fusion.

A single barometric pressure sensor is used for altitude estimation and vertical rate computation. Its data complements inertial measurements and is especially relevant for low-frequency altitude tracking and apogee detection. The barometer is shared as a common input across estimation and logging subsystems.

For data persistence, the system integrates non-volatile flash memory to support continuous onboard data logging during flight. This storage is intended for high-reliability recording of critical sensor data, and system states. In addition, a physical SD card slot is included to allow retrieval of detailed flight data after recovery.

Finally, the system incorporates LED indicators and a buzzer to provide immediate visual and audible feedback regarding system states such as power-up, arming status, fault conditions, and mode transitions.


## Architectures
### Option 1 - Dedicated PMC and Dual OBC
This architecture is built around two identical battery packs, routed through a Power Management Controller (PMC) that acts as the sole authority over power supervision and distribution. Although the packs are electrically identical, their usage is differentiated by design: one pack is primarily associated with the actuator power domain, while the other supplies the onboard computers (OBCs). Only one pack per domain is active at a time, with selection and protection handled according to domain criticality.

The PMC, implemented on a dedicated microcontroller, continuously monitors both identical battery packs by measuring voltage, current, and optionally temperature. In addition to pack-level supervision, it monitors the electrical consumption of each actuator channel individually.

For the actuator power domain, the two identical battery packs are routed through PMC-controlled electronic power switches. The PMC firmware explicitly selects which actuator battery pack is connected to the actuator bus at any given time, and can fully disconnect or switch between packs as needed. This allows the PMC to actively choose the power source for the actuators and to isolate a pack in response to excessive current draw, detected faults, degradation, or higher-level system states.

Downstream of the selected actuator pack, each actuator channel is further gated by electronically controlled switches, also commanded by the PMC. This enables per-channel activation, isolation, and load shedding, ensuring that both pack selection and individual actuator enablement are under direct PMC control.

In contrast, the battery pack supplying the onboard computers is governed primarily by hardware supervision rather than firmware logic. Undervoltage, overcurrent, and brown-out protection are implemented using analog hardware supervisors capable of removing or restoring power independently of PMC firmware execution. The PMC passively observes and reports the state of this OBC power rail, but it does not act as the sole decision-maker. This design ensures that OBC power behavior remains deterministic, bounded, and resistant to software failure, even if the PMC firmware becomes compromised.

Above the PMC operate two onboard computers with clearly separated responsibilities.

OBC #1 (Primary Control OBC) is dedicated exclusively to running control algorithms and robust state estimation. Its design philosophy assumes that actuator power channels are available, not something it actively manages. When channels are enabled by the PMC, OBC #1 simply outputs control signals to the actuators. It does not perform power arbitration, logging, communication, or sensor preprocessing. Its operation depends on receiving validated sensor data and system status from OBC #2.

OBC #2 (Secondary / Data and Services OBC) is responsible for all sensor interfacing and data handling. It acquires raw sensor data, processes and conditions it, runs secondary or complementary state estimation algorithms, and stores data for later retrieval. It also handles non-critical system services such as data logging, Bluetooth communication with an external application. OBC #2 acts as a data provider to OBC #1, delivering sensor information and derived state estimates on demand. In this architecture, OBC #2 is also responsible for communication with the PMC, collecting power status, fault flags, and battery telemetry.

This architecture enforces strict separation: the PMC controls power, OBC #1 controls behavior, and OBC #2 manages data and interaction.

### Option 2 - Dual OBC Architecture Without PMC

Architecture 2 removes the dedicated PMC while keeping the two-OBC structure and the concept of identical battery packs. The packs still exist as separate power sources, but they are no longer routed through an intelligent power authority. Instead, battery protection is limited to passive hardware elements such as fuses, current limiters, and voltage regulators.

Without the PMC, no component has authoritative control over power distribution. Battery pack selection is hardware configured, and no actuator channels can be actively enabled or disabled at runtime based on power conditions. Power-related decisions are reduced to monitoring on pack status.

The Primary Control OBC (OBC #1) retains its focused role: running control algorithms and robust state estimation. It continues to assume that actuator power channels are available, and it outputs control signals accordingly. 

The Secondary OBC (OBC #2) monitors battery voltages and currents and continues to handle sensor processing, data storage, logging, and communication (including Bluetooth). It reports battery status and warnings to OBC #1, but it cannot directly isolate faults or remove power from loads. Communication between the two OBCs remains similar to Architecture 1, but without the PMC, fault handling becomes slower, less deterministic, and entirely software-dependent.

Compared to Architecture 1, this design reduces hardware complexity but sacrifices independent power authority and fast fault isolation.

### Option 3 - Single-OBC

Architecture 3 collapses the responsibilities of Architecture 2 into a single onboard computer. The system still uses one or two identical battery packs, but domain separation is minimal, and power protection relies almost entirely on passive hardware and basic software checks.

In this architecture, the single OBC performs control, state estimation, sensor acquisition, and basic battery monitoring within a single firmware context. Sensor data is processed locally, and only essential state estimation algorithms are executed. Non-essential services are removed: Bluetooth communication and higher-level interaction features are not included.

Battery monitoring is simplified to a reduced set of measurements, typically at lower sampling rates. There is no independent supervisory layer, no secondary data-processing unit, and no authoritative power control. Any software fault or overload directly impacts all system functions simultaneously.

This architecture prioritizes minimal hardware, fast development, and simplicity. Relative to Architectures 1 and 2, it offers the least robustness and the weakest fault isolation, but it may be acceptable for early prototypes or low-risk applications.