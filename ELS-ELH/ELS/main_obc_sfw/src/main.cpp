#include <Arduino.h>
#include "constants.h"
#include "SPI_&_I2C.h"
#include "sensors_manager.h"
#include "error_warning.h"
#include "LEDs_Buzzer_&_signals.h"
#include <SoftwareSerial.h>

float previousSecs = 0;
float previousTelemetry = 0;
float currentSecs;


void setup() {
  Serial.begin(BAUD_RATE);
  Serial.println("ELS INITIALIZED");
  InitBNO055();
  InitBME280();
  InitUblox();
  InitMPU9250();
}

void loop() {
  currentSecs = millis() / 1000.0;
  if (currentSecs - previousTelemetry >= 1.0) {
    previousTelemetry = currentSecs;
    ReadMPU9250();
    ReadBNO055();
    ReadBME280();
    ReadUblox();
  }

}