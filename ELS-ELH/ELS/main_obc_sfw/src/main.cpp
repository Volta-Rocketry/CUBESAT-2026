#include <Arduino.h>
#include "constants.h"
#include "sensors_manager.h"

float previous_secs = 0;
float previous_telemetry = 0;
float current_secs;

void setup() {
  Serial.begin(BAUD_RATE);
  Serial.println("ELS-02 INITIALIZED");
  InitMPU9250();
  InitBNO055();
  InitBME280();
  InitUblox();
  InitTransducers();
}

void loop() {
  current_secs = millis() / 1000.0;

}