#pragma once

#include "constants.h"

void InitMPU9250();
void InitBNO055();
void InitBME();
void InitUblox();
void InitTransducers();
void InitSolenoidValves();

void calibrateSensors();

void readSensors();
