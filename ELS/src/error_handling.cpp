#include "error_handling.h"

#include <Arduino.h>

void criticalError(const char* message) {
    Serial.println(message);
}