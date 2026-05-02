#include "error_warning.h"

#include <Arduino.h>
#include "constants.h"
#include "signals.h"
#include "BluetoothSerial.h"

extern BluetoothSerial SerialBT;


void CriticalErrorSensor(const char* messages) {
    Serial.print("Critical [ERROR]: ");
    Serial.println(messages);
    SerialBT.print("Critical [ERROR]: ");
    SerialBT.println(messages);

    uint32_t previousErrorCountMilis = millis();
    uint8_t blinkCount = 0;
    while (blinkCount < 6) {
        uint32_t ErrorCountMilis = millis();
        if (ErrorCountMilis - previousErrorCountMilis >= 500) {
            previousErrorCountMilis = ErrorCountMilis;
            ColorRGB(255, 0, 0);
            PWMBuzzer(2500, 150);
            blinkCount++;
        }
    }
}

