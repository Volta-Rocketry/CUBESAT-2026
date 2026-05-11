#pragma once

#include <Arduino.h>
#include <stdint.h>
#include "flight_computer.h"
#include "constants.h"

uint16_t crc16CCITT(const uint8_t* data, uint16_t length);
bool commsInit(HardwareSerial& serialPort, int rxPin, int txPin, const CommsInitData* d);
void commsTick();

void commsUpdateCTR(uint32_t timestamp,
                    float altitude, float vertical_velocity,
                    float ax, float ay, float az,
                    float gx, float gy, float gz,
                    float qw, float qx, float qy, float qz,
                    FlightState state);

void commsUpdateCAM(uint32_t timestamp,
                    float ax, float ay, float az,
                    float gx, float gy, float gz);

void commsUpdateFlight(uint32_t timestamp,
                    float altitude,
                    FlightState state);
