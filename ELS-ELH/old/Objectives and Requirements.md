# ELH-ELS (Electronics Hardware and Software) Objectives and Requirements

## Objectives
### General Objective
Develop an electronics platform to support control, power, and payload operations required for 2026 Volta´s Cubesat mission.
### Specific Objectives
1. Provide electrical power availability to enable nominal operation of all primary subsystems.
2. Support attitude determination and control activities during descent.
3. Enable system-level awareness of mission flight phases for operational decision-making.
4. Ensure system operability through defined modes, including startup, nominal operation, contingency, and safe states.
5. Enable reliable data handling, including acquisition, routing, storage, and access for onboard and ground use.
6. Enable verification and validation of system behavior through observable states and testable interfaces.

## Success Criteria
### Objective 1
- The objective is considered successful if all primary subsystems remain powered and operational throughout all defined operational modes without unintended resets or power interruptions.
### Objective 2
- The objective is considered successful if attitude state information is continuously available to the control logic throughout the descent phase without loss of estimation.
### Objective 3
- The objective is considered successful if the system correctly transitions between defined flight phases in a manner consistent with the mission timeline and external events.
- The objective is considered successful if the current flight phase is externally observable and unambiguous at all times.
### Objective 4
- The objective is considered successful if the system can enter, operate within, and exit all defined operational modes without manual intervention.
- The objective is considered successful if the system can transition to a safe or contingency mode in response to anomalous conditions.
### Objective 5
- The objective is considered successful if mission-critical data is correctly acquired, routed, and made available to onboard consumers when required.
- The objective is considered successful if stored data remains accessible and uncorrupted across operational modes and power cycles.
- The objective is considered successful if relevant data can be accessed at ground after operation
### Objective 6
- The objective is considered successful if system states, transitions, and key behaviors are externally observable through defined interfaces.
- The objective is considered successful if system behavior can be verified through repeatable tests conducted in representative operational conditions.
- The objective is considered successful if test results can be traced to specific objectives and system behaviors.

## System Requirements
- The system shall detect and record power-related anomalies that affect subsystem operation..
- The system shall provide attitude state information required for control functions during the descent phase.
- The system shall support execution of attitude control actions during descent.
- The system shall detect and report loss or degradation of attitude information.
- The system shall maintain an internal representation of the current mission flight phase.
- The system shall transition between defined flight phases based on mission events or conditions.
- The system shall expose the current flight phase as an externally observable state.
- The system shall support startup, nominal, contingency, and safe operational modes.
- The system shall transition between operational modes without manual intervention.
- The system shall maintain basic operational functionality in safe mode.
- The system shall maintain basic operational functionality in safe mode.
- The system shall store mission-critical data for later access.
- The system shall preserve data integrity across operational modes and power cycles.
- The system shall support monitoring of key system behaviors during operation.

## External and Imposed Requirements
- The system shall not include LIPO batteries
- The system shall provide data from the IMU and the Altimeter to the camera subsystem on demand
- The system shall provide functionality to two electrovalves operating at 12V with an aproximate power consumption of 7.6 W.
- The system shall provide functionality to one analog pressure transductor operating at 5V