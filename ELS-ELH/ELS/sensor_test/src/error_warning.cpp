#include "error_warning.h"

#include <Arduino.h>
#include "constants.h"

void CriticalErrorSensor(const char* messages) {
    Serial.print(" [ERROR] critical : ");
    Serial.println(messages);
    while (true) {
        digitalWrite(LED_RED_PIN, HIGH);
        digitalWrite(BUZZER_PIN, HIGH);
    }
}
