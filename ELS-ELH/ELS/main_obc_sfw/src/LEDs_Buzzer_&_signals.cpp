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
    int blink_count = 0;
    float previous_secs = 0;
    bool led_state = LOW;
    float secs_PCB = millis() / 1000.0;
    if (blink_count < 6) {
        if (secs_PCB - previous_secs >= 1.0) {
            previous_secs = secs_PCB;
            led_state = !led_state;
            digitalWrite(LED_RED_PIN, led_state);

            blink_count++;
        }
    }
}