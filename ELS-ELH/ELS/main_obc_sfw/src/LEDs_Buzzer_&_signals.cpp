#include "LEDs_Buzzer_&_signals.h"

#include <Arduino.h>
#include "constants.h"

void InitLEDBuzzerButton() {
    // Code to initialize LEDs and buzzer
    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_BLUE_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    Serial.println("LEDs and buzzer initialized successfully.");
    digitalWrite(LED_RED_PIN, LOW);
    digitalWrite(LED_GREEN_PIN, LOW);  
    digitalWrite(LED_BLUE_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
}

void InitLedPCB() {
    int blinkCount = 0;
    uint32_t previousMilis = 0;
    bool ledState = LOW;
    uint32_t milisPCB = millis();
    while (blinkCount < 6) {
        if (milisPCB - previousMilis >= 1000) {
            previousMilis = milisPCB;
            ledState = !ledState;
            digitalWrite(LED_BLUE_PIN, ledState);

            blinkCount++;
        }
    }
}