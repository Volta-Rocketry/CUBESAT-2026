#pragma once

#include "constants.h"
#include "error_warning.h"

void initLedBuzzerActuators();
void initMPU6050();
void initQMC5883L();
void initBMP180();
void initBNO055();
void initBME280();
void initUblox();
void initActuators1();
void initActuators2();

void calibrateSensors();
void calibrateMagnetometer();

void readMPU6050();
void readQMC5883L();
void readBMP180();
void readBNO055();
void readBME280();
void readUblox();

void openActuators1Voltage();
void closeActuators1Voltage();
void openActuators2Voltage();
void closeActuators2Voltage();