#include <Arduino.h>
#include "constants.h"
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
  currentSecs = millis() / 1000.0;
  InitLedPCB();

}