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

  //InitLEDBuzzerButton();

  //InitLedPCB();

  InitBNO055();
  InitBME280();
  InitUblox();
  //InitMPU9250();

  flight_computer_init();

  Serial.println("CUBESAT INITIALIZED");

  CalibrateSensors();
}

void loop() {
  currentMilis = millis();
<<<<<<< HEAD
  if (currentMilis - previousTelemetry >= 1000) {
    previousTelemetry = currentMilis;
    ReadMPU9250();
    ReadBNO055();
    ReadBME280();
    ReadUblox();
=======
  if (numCalib == 2) {
    if (currentMilis - previousTelemetry >= 1000) {
      previousTelemetry = currentMilis;
      //ReadMPU9250();
      ReadBNO055();
      ReadBME280();
      ReadUblox();
  }
>>>>>>> 701423abe53df5a795626c8311f21f80fd24adff
  }

}