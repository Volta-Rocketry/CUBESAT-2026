#pragma once

#include "constants.h"
#include "error_warning.h"

extern int numCalib;

void InitExtencionBoard();

void InitMPU6050();
void InitHMC5883();
void InitBMP180();
void InitBNO055();
void InitBME280();
void InitUblox();
void InitActuators();

void CalibrateSensors();
void CalibratMagnetometer();

void ReadMPU6050();
void ReadHMC5883();
void ReadBMP180();
void ReadBNO055();
void ReadBME280();
void ReadUblox();

void OpenActuatorsVoltage();
void CloseActuatorsVoltage();