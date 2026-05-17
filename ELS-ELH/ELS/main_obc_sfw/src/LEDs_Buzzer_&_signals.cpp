#include "LEDs_Buzzer_&_signals.h"

#include <Arduino.h>
#include "constants.h"
#include <Adafruit_PCF8574.h>

extern Adafruit_PCF8574 pcf;

void InitPCB() {
    uint8_t blinkCount = 0;
    bool ledState = LOW;
    uint16_t previousMilis = millis();
    while (blinkCount < 6) {
        uint16_t currentMilis = millis();
        if (currentMilis - previousMilis >= 1000) {
            previousMilis = currentMilis;
            ledState = !ledState;
            pcf.digitalWrite(LED_BLUE_PIN, ledState);

            blinkCount++;
        }
    }
    Serial.println("PCB initialized successfully.");
}
/*
void PlayBuzzerTone(long frequence, long duration){
    static unsigned long startMilis = 0, lastMicros = 0;
    static long halfPeriod = 0;
    static bool state = HIGH;

    if (frequence > 0) {
        halfPeriod = 1000000 / (frequence * 2);
        startMilis = millis();
        return; 
    }
    else if (duration == 0) {
        return; 
    }

    if (millis() - startMilis >= duration) {
        duration = 0;
        pcf.digitalWrite(BUZZER_PIN, HIGH); 
        return;
    }

    if (micros() - lastMicros >= halfPeriod) {
        lastMicros = micros();
        state = !state;
        pcf.digitalWrite(BUZZER_PIN, state);
    }
}
    */

    void PlayBuzzerTone(long frequency, long duration) {
    if (frequency == 0) return;
    long halfPeriod = 1000000L / (frequency * 2);
    long loops = (duration * 1000L) / (halfPeriod * 2);
    for (long i = 0; i < loops; i++) {

        pcf.digitalWrite(BUZZER_PIN, LOW);
        delayMicroseconds(halfPeriod);
        
        pcf.digitalWrite(BUZZER_PIN, HIGH);
        delayMicroseconds(halfPeriod);
    }
    
    pcf.digitalWrite(BUZZER_PIN, HIGH);
}

