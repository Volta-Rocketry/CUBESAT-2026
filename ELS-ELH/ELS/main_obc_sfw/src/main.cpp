#include <Arduino.h>
#include "constants.h"
#include "sensors_manager.h"
#include "error_warning.h"
#include "LEDs_Buzzer_&_signals.h"

float previous_secs = 0;
float previous_telemetry = 0;
float current_secs;

void setup() {
  Serial.begin(BAUD_RATE);
  Serial.println("ELS-02 INITIALIZED");
  InitLEDBuzzer();
  InitMPU9250();
  InitBNO055();
  InitBME280();
  InitUblox();
  InitTransducers();
  InitActuators();
  CloseActuatorsVoltage();
}

void loop() {
  current_secs = millis() / 1000.0;
  InitLedPCB();

}