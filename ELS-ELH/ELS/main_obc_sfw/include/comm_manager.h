#pragma once

#include <stdint.h>
#include "flight_computer.h"
#include "constants.h"

void CommsInit();
void CommsTick();

//Deberia pasar los structs a constants?
typedef struct {
    uint32_t timestamp;
    float altitude;
    float vertical_velocity;
    float ax, ay, az;
    float gx, gy, gz;
    float qw, qx, qy, qz;
    FlightState flight_state;
} CommsCtrData;

typedef struct {
    uint32_t timestamp;
    float ax, ay, az;
    float gx, gy, gz;
} CommsCamData;

typedef struct {
    uint32_t timestamp;
    float altitude;
    FlightState flight_state;
} CommsFlightData;

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
