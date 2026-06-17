#pragma once
#include <Arduino.h>
#include <string.h>
#include <stdint.h>
#include "BluetoothSerial.h"

extern BluetoothSerial SerialBT;

// =========================== CONSTANTS AND DEFINITIONS ===========================

// ──────────── UART BUS ────────────
#define CTR_TX 23
#define CTR_RX 5

// ──────────── GPIOs ────────────
#define RED_LED  13
#define GREEN_LED 33
#define BLUE_LED 32

#define BUZZER 4

#define R_PWM_MOTOR 25
#define R_EN_MOTOR 27
#define L_PWM_MOTOR 26
#define L_EN_MOTOR 14
#define L_IS_MOTOR 35
#define R_IS_MOTOR 34

// ===========================================================================

// =========================== DATA DEFINITIONS ===========================
typedef enum {
    STATE_IDLE,
    STATE_INIT,
    STATE_PAD,
    STATE_ASCENT,
    STATE_EJECTION,
    STATE_CONTROL,
    STATE_CUTOFF,
    STATE_RECOVERY
} FlightState;

struct CommsCtrData {
    uint32_t timestamp;          // Timestamp
    float altitude;              // Altitude BME280
    float vertical_velocity;     // Veritcal velocity
    float ax, ay, az;            // Acceleration BNO055
    float gx, gy, gz;            // Angular velocity BNO055
    float qw, qx, qy, qz;        // Cuaternion BNO055
    FlightState flight_state;    // Estado de vuelo
};

struct RWController
{
    float kp;
    float kd;

    float prev_error;
    uint32_t prev_time;
};

extern RWController rwController;

