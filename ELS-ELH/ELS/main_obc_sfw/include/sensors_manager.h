#pragma once

#include "constants.h"
#include "error_warning.h"

void InitMPU9250();
void InitBNO055();
void InitBME();
void InitUblox();
void InitTransducers();
void InitSolenoidValves();

void CalibrateSensors();

void ReadSensors();

void ReadMPU9250();
void ReadBNO055();
void ReadBME();
void ReadUblox();
void ReadTransducers();
void ReadSolenoidValves();