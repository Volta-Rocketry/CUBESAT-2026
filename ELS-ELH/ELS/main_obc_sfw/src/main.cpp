#include <Arduino.h>
#include "constants.h"
#include "sensors_manager.h"
#include "error_warning.h"
#include "LEDs_Buzzer_&_signals.h"
#include "flight_computer.h"
#include <SoftwareSerial.h>

uint32_t previousTelemetry = 0;
uint32_t currentMilis;


void setup() {
    Serial.begin(BAUD_RATE);
    InitExtencionBoard();

    Serial.println("CUBESAT-1 Flight Computer Starting...");
}

void loop() {

}