#include "error_warning.h"

#include <Arduino.h>

void criticalError(const char* message) {
    Serial.println(" [ERROR] critical : ");
    Serial.println(message);
}