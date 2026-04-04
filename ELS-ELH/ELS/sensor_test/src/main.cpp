#include <Arduino.h>
#include "constants.h"
#include "sensors_manager.h"
#include "error_warning.h"
#include <SoftwareSerial.h>

float previousSecs = 0;
float previousTelemetry = 0;
float currentSecs;
bool pinBlueState = HIGH;


void setup() {
  Serial.begin(BAUD_RATE);
  Serial.println("ELS INITIALIZED");
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  InitMPU9250();
  InitBNO055();
  InitBME280();
  InitUblox();
}

void loop() {
  currentSecs = millis() / 1000.0;
  Serial.println(currentSecs);
  if (currentSecs - previousSecs >= 1) {
    previousSecs = currentSecs;
    digitalWrite(LED_BLUE_PIN, pinBlueState);
    pinBlueState = !pinBlueState;
  }

}