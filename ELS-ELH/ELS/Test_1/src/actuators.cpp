#include "actuators.h"

#include "Arduino.h"

void InitTransducers() {
    pinMode(PIN_TRANSDUCER, INPUT);
    Serial.println("Transducer initialized");

}
void InitValves() {
    pinMode(PIN_VALVE_1, OUTPUT);
    pinMode(PIN_VALVE_2, OUTPUT);
    
    CloseValve1();
    CloseValve2();
    Serial.println("Valves initialized and closed");
}
void OpenValve1() {
    digitalWrite(PIN_VALVE_1, HIGH);

}
void CloseValve1() {
    digitalWrite(PIN_VALVE_1, LOW);
}
void OpenValve2() {
    digitalWrite(PIN_VALVE_2, HIGH);
}
void CloseValve2() {
    digitalWrite(PIN_VALVE_2, LOW);
}

float ReadTransducer() {
    int transducerValue = analogRead(PIN_TRANSDUCER);
    float voltage = transducerValue * (5.0 / 1023.0);
    float pressureMpa = ((voltage - 0.5) / 4.0) * 0.5;
    if (pressureMpa < 0.0) {
            pressureMpa = 0.0;
        }
    return pressureMpa;
}