#include <Arduino.h>
#include "constants.h"
#include "sensors_manager.h"
#include "error_warning.h"
#include "signals.h"
#include "flight_computer.h"
#include <Adafruit_PCF8574.h>
#include <SoftwareSerial.h>
#include "BluetoothSerial.h"
#include "comm_manager.h"

Adafruit_PCF8574 pcf;
BluetoothSerial SerialBT;

uint32_t previousTelemetry = 0;
uint32_t currentMilis;

void setup() {
    Serial.begin(BAUD_RATE);
    SerialBT.begin("ESP32_Robot"); 
    Serial.println("El dispositivo ya se puede emparejar vía Bluetooth.");

    InitExtencionBoard();
    InitMPU6050();
    InitQMC5883L();
    InitBMP180();
    InitBNO055();
    flight_computer_init();
    InitBME280();
    InitUblox();
    comms_init();
    Serial.println("CUBESAT-1 Flight Computer Starting...");
    InitPCB();
}

void loop() {
    currentMilis = millis();
      comms_tick();
    if (currentMilis - previousTelemetry >= 1000) {
        previousTelemetry = currentMilis;
        Serial.println("Collecting telemetry data...");
        SerialBT.println("HOLA");
        delay(2000);
        if (SerialBT.available() >0 ){
            String command = SerialBT.readStringUntil('\n');
            command.trim();
            Serial.println(command);
            if (command == "LED") {
                pcf.digitalWrite(LED_BLUE_PIN, HIGH);
                SerialBT.println("LED blue");
                Serial.println("LED BLUE");
            }
            else if (command == "A1"){
                OpenActuators1Voltage();
            }
            else if (command == "A2"){
                OpenActuators2Voltage();
            }
            else if (command == "C1"){
                CloseActuators1Voltage();
            }
            else if (command == "C2"){
                CloseActuators2Voltage();
            }
        }
    }
    // flight_computer_update();
}
