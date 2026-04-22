#include <Arduino.h>

HardwareSerial MySerial(1);

void setup() {
    Serial.begin(115200);

    // UART1: RX=36, TX=25
    MySerial.begin(115200, SERIAL_8N1, 36, 25);

    Serial.println("ESP2 Recibiendo...");
}

void loop() {
    if (MySerial.available()) {
        String mensaje = MySerial.readStringUntil('\n');
        mensaje.trim();

        Serial.println("Recibido: " + mensaje);
    }
}