#include <Arduino.h>
#include "constants.h"
#include "sensors_manager.h"
#include "error_warning.h"
#include "LEDs_Buzzer_&_signals.h"
#include "flight_computer.h"
#include <SoftwareSerial.h>
#include <Adafruit_PCF8574.h>

Adafruit_PCF8574 pcf;

uint32_t previousTelemetry = 0;
uint32_t currentMilis;

void setup() {
    Serial.begin(BAUD_RATE);
    InitExtencionBoard();
    InitBNO055();
    InitMPU9250();
    InitBME280();
    InitUblox();

    Serial.println("CUBESAT-1 Flight Computer Starting...");
    InitPCB();
}

void loop() {
    currentMilis = millis();
    if (currentMilis - previousTelemetry >= 1000) {
        previousTelemetry = currentMilis;
        Serial.println("Collecting telemetry data...");
        // Aquí puedes llamar a las funciones para leer los sensores y enviar los datos
        PlayBuzzerTone(1000, 200);
        delay(2000);
        PlayBuzzerTone(2000, 200);
        delay(2000);
    }
}