#pragma once

#include "constants.h"
#include "error_warning.h"

void InitMPU9250();
void InitBNO055();
void InitBME280();
void InitUblox();
void InitTransducers();
void InitActuators();

void CalibrateSensors();
void CalibratMagnetometer();

void ReadMPU9250();
void ReadBNO055();
void ReadBME280();
void ReadUblox();
void ReadTransducers();

void OpenActuatorsVoltage();
void CloseActuatorsVoltage();