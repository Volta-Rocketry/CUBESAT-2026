#include "error_warning.h"

#include <Arduino.h>
#include "constants.h"
#include <Adafruit_PCF8574.h>
#include "BluetoothSerial.h"

extern BluetoothSerial SerialBT;
extern Adafruit_PCF8574 pcf;


void CriticalErrorSensor(const char* messages) {
    Serial.print("Critical [ERROR]: ");
    Serial.println(messages);
    SerialBT.print("Critical [ERROR]: ");
    SerialBT.println(messages);
    uint16_t previousErrorCountMilis = millis();
    uint8_t blinkCount = 0;
    bool state = LOW;
    while (blinkCount < 6) {
        uint16_t ErrorCountMilis = millis();
        if (ErrorCountMilis - previousErrorCountMilis >= 500) {
            previousErrorCountMilis = ErrorCountMilis;
            state = !state;
            pcf.digitalWrite(LED_RED_PIN, state);
            pcf.digitalWrite(BUZZER_PIN, !state);
            blinkCount++;
        }
    }

}

