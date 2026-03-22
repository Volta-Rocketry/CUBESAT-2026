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








