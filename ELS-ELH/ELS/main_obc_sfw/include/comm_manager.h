#pragma once

#include <Arduino.h>
#include <stdint.h>
#include "flight_computer.h"
#include "constants.h"

uint16_t crc16_ccitt(const uint8_t* data, uint16_t length);
bool CommsInit(HardwareSerial& serialPort, int rxPin, int txPin, const CommsInitData* d);
void CommsTick();

void CommsUpdateCTR(uint32_t timestamp,
                    float altitude, float vertical_velocity,
                    float ax, float ay, float az,
                    float gx, float gy, float gz,
                    float qw, float qx, float qy, float qz,
                    FlightState state);

void CommsUpdateCAM(uint32_t timestamp,
                    float ax, float ay, float az,
                    float gx, float gy, float gz);

void CommsUpdateFlight(uint32_t timestamp,
                    float altitude,
                    FlightState state);
