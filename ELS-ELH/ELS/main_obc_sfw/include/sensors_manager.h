#pragma once

#include "constants.h"
#include "error_warning.h"

void InitMPU9250();
void InitBNO055();
void InitBME280();
void InitUblox();
void InitTransducers();

void CalibrateSensors();

void ReadMPU9250();
void ReadBNO055();
void ReadBME280();
void ReadUblox();
void ReadTransducers();
