// ──────────── LIBRERIAS ────────────

#include "comm_manager.h"
#include <Arduino.h>
#include <string.h>

// ──────────── VARIABLES LOCALES ────────────


static uint32_t s_last_flight_send_ms = 0;

// ──────────── FUNCIONES LOCALES ────────────

// envío de los paquetes

static void send_flight_frame() {
    Serial1.println("FUNCIONA"); 
}

// ──────────── FUNCIONES GLOBALES ────────────

// inicialización de puertos UART

void comms_init() {

    Serial1.begin(COMMS_BAUD_RATE, SERIAL_8N1, FLIGHT_RX_PIN, FLIGHT_TX_PIN);

    s_last_flight_send_ms = 0;

    Serial.println("[EPA] inicializada la comm UART.");
}

// reloj maestro de envío

void comms_tick() {

    uint32_t now = millis();

    if (now - s_last_flight_send_ms >= FLIGHT_SP_INTERVAL_MS) {
        s_last_flight_send_ms = now;
        send_flight_frame();
    }
}