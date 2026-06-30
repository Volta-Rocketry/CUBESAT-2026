#include "error_warning.h"

#include <Arduino.h>
#include "constants.h"
#include "signals.h"
#include "BluetoothSerial.h"

extern BluetoothSerial SerialBT;


void criticalErrorSensor(const char* messages) {
    Serial.print("Critical [ERROR]: ");
    Serial.println(messages);
    SerialBT.print("Critical [ERROR]: ");
    SerialBT.println(messages);
    uint8_t blinkCount = 0;
    while (blinkCount < 3) {
        colorRGB(255, 0, 0);
        PWMBuzzer(2500, 150);
        delay(BLINK);
        colorRGB(0, 0, 0);
        PWMBuzzer(0, 0);
        delay(BLINK);
        blinkCount++;
    }
}

