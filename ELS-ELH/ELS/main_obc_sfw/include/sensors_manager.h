#pragma once

#include "constants.h"
#include "error_warning.h"

void InitLedBuzzerActuators();
void InitMPU6050();
void InitQMC5883L();
void InitBMP180();
void InitBNO055();
void InitBME280();
void InitUblox();
void InitActuators1();
void InitActuators2();

void CalibrateSensors();
void CalibratMagnetometer();

void ReadMPU6050();
void ReadQMC5883L();
void ReadBMP180();
void ReadBNO055();
void ReadBME280();
void ReadUblox();

void OpenActuators1Voltage();
void CloseActuators1Voltage();
void OpenActuators2Voltage();
void CloseActuators2Voltage();