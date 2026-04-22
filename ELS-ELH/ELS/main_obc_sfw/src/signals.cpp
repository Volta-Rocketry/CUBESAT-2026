#include "signals.h"

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
}

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

