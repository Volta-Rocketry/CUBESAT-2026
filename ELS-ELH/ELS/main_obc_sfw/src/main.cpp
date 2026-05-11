#include <Arduino.h>
#include "constants.h"
#include "sensors_manager.h"
#include "error_warning.h"
#include "signals.h"
#include "flight_computer.h"
#include <SoftwareSerial.h>
#include "BluetoothSerial.h"
#include "comm_manager.h"

BluetoothSerial SerialBT;

uint32_t previousTelemetry = 0;
uint32_t currentMilis;

void setup() {
    Serial.begin(BAUD_RATE);
    SerialBT.begin("ESP32_Robot"); 
    initLedBuzzerActuators();
    initPCB();
    flightComputerInit();
    initPCB();
}

void loop() {
    currentMilis = millis();
}
