#pragma once

#include <stdint.h>

// Baud Rate
#define BAUD_RATE 115200

// Time intervals
#define SENSOR_READ_INTERVAL 1

// PIN definition
#define PIN_VALVE_1 2
#define PIN_VALVE_2 3
#define PIN_TRANSDUCER A0

// PIN HC-05
#define PIN_HC05_TX 10
#define PIN_HC05_RX 11

// Communication constants
#define ID "D"
#define OPEN_VALVE_1 "A1"
#define CLOSE_VALVE_1 "S1"
#define OPEN_VALVE_2 "A2"
#define CLOSE_VALVE_2 "S2"
#define SEND_SIGNAL "R"