#include "LEDs_Buzzer_&_signals.h"

#include <Arduino.h>
#include "constants.h"

void InitLEDBuzzer() {
    // Code to initialize LEDs and buzzer
    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_BLUE_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    Serial.println("LEDs and buzzer initialized successfully.");
}

void InitLedPCB() {
    int blinkCount = 0;
    float previousSecs = 0;
    bool ledState = LOW;
    float secsPCB = millis() / 1000.0;
    if (blinkCount < 6) {
        if (secsPCB - previousSecs >= 1.0) {
            previousSecs = secsPCB;
            ledState = !ledState;
            digitalWrite(LED_RED_PIN, ledState);

            blinkCount++;
        }
    }
}