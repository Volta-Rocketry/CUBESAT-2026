#include <Arduino.h>
#include "constants.h"
#include "sensors_manager.h"

float previous_secs = 0;
float previous_telemetry = 0;
bool led_state = false;

void setup() {
  Serial.begin(BAUD_RATE);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("ELS-02 INITIALIZED");
  InitMPU9250();
  InitBNO055();
  InitBME();
  InitUblox();
  InitTransducers();
  InitSolenoidValves();
}

void loop() {
  float current_secs = millis() / 1000.0;

  if (current_secs - previous_secs > BLINK_S) {
    previous_secs = current_secs;
    led_state = !led_state;
    digitalWrite(LED_PIN, led_state);
  }

}