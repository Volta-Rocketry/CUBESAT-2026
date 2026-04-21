// ──────────── HEADER ────────────

#pragma once

// ──────────── LIBRERIAS ────────────

#include <stdint.h>

// ──────────── PINES Y PARÁMETROS ────────────

#define ID_FLIGHT_SP 0X20
#define COMM_SYNC_1  0xAA
#define COMM_SYNC_2  0x55
//#define FLIGHT_SP_PAYLOAD_LEN 9
//#define FLIGHT_SP_FRAME_SIZE   (2 + 1 + 1 + FLIGHT_SP_PAYLOAD_LEN + 2)
#define FLIGHT_SP_INTERVAL_MS   1000
#define COMMS_BAUD_RATE  115200
#define FLIGHT_TX_PIN    17
#define FLIGHT_RX_PIN    16

// ──────────── FUNCIONES GLOBALES ────────────

void comms_init();
void comms_tick();

/*
// ──────────── HEADER ────────────

#pragma once

// ──────────── LIBRERIAS ────────────

#include <stdint.h>
#include "flight_computer.h"

// ──────────── PINES Y PARÁMETROS ────────────

#define ID_CTR_TP    0x10
#define ID_CAM_TP    0x11
#define ID_FLIGHT_SP 0X20
#define COMM_SYNC_1  0xAA
#define COMM_SYNC_2  0x55
#define CTR_TP_PAYLOAD_LEN   53
#define CAM_TP_PAYLOAD_LEN   28
#define FLIGHT_SP_PAYLOAD_LEN 9
#define CTR_TP_FRAME_SIZE   (2 + 1 + 1 + CTR_TP_PAYLOAD_LEN + 2)
#define CAM_TP_FRAME_SIZE   (2 + 1 + 1 + CAM_TP_PAYLOAD_LEN + 2)
#define FLIGHT_SP_FRAME_SIZE   (2 + 1 + 1 + FLIGHT_SP_PAYLOAD_LEN + 2)
#define CTR_TP_INTERVAL_MS   10
#define CAM_TP_INTERVAL_MS   10
#define FLIGHT_SP_INTERVAL_MS   1000
#define COMMS_BAUD_RATE   115200
#define CTRL_OBC_TX_PIN   43
#define CTRL_OBC_RX_PIN   44
#define CAM_OBC_TX_PIN    15
#define CAM_OBC_RX_PIN    16

// ──────────── ESTRUCTURAS FRECUENCIA ────────────

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

// ──────────── FUNCIONES GLOBALES ────────────

void comms_init();

void comms_update_ctr(uint32_t timestamp,
                    float altitude, float vertical_velocity,
                    float ax, float ay, float az,
                    float gx, float gy, float gz,
                    float qw, float qx, float qy, float qz,
                    FlightState state);

void comms_update_cam(uint32_t timestamp,
                    float ax, float ay, float az,
                    float gx, float gy, float gz);

void comms_update_flight(uint32_t timestamp,
                    float altitude,
                    FlightState state);

void comms_tick();
*/