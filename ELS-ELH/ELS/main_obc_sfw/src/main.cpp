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

void setup() {
    Serial.begin(BAUD_RATE);
    hspi.begin(18, 19, 23, -1);
    delay(100);
    SerialBT.begin("ESP32_Robot"); 
    initLedBuzzerActuators();
    initPCB();
    flightComputerInit();
    initPCB();
}

void loop() {
    flightComputerUpdate();
    delay(1);
}
