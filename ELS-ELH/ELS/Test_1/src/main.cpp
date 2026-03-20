#include <Arduino.h>

#include "constants.h"
#include "actuators.h"

float pressure;
float lastSensorReadTime = 0;
String command;
bool valveState1;
bool valveState2;
int valve1;
int valve2;

void setup() {
  Serial.begin(BAUD_RATE);
  InitTransducers();
  InitValves();
}

void loop() {
  float currentTime = millis()/1000;
  if (Serial.available() > 0){
    command = Serial.readStringUntil('\n');
    command.trim();
    if (command == OPEN_VALVE_1) {
      OpenValve1();
      Serial.println("Valve 1 opened");
      valveState1 = true;
      valve1 = 1;
    } 
    else if (command == CLOSE_VALVE_1) {
      CloseValve1();
      Serial.println("Valve 1 closed");
      valveState1 = false;
      valve1 = 0;
    } 
    else if (command == OPEN_VALVE_2) {
      OpenValve2();
      Serial.println("Valve 2 opened");
      valveState2 = true;
      valve2 = 1;
    } 
    else if (command == CLOSE_VALVE_2) {
      CloseValve2();
      Serial.println("Valve 2 closed");
      valveState2 = false;
      valve2 = 0;
    } 
    else if (command == SEND_SIGNAL) {
      Serial.print("M");
      Serial.println("I'm alive");
    } 
    else {
      Serial.println("Unknown command received");
    }
  }
  if (currentTime - lastSensorReadTime >= SENSOR_READ_INTERVAL) {
    lastSensorReadTime = currentTime;
    pressure = ReadTransducer();
    Serial.print(ID);
    Serial.print(",");
    Serial.print(pressure);
    Serial.print(",");
    Serial.print(valve1);
    Serial.print(",");
    Serial.println(valve2);
  }
}
