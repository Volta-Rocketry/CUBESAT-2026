#include <Arduino.h>

HardwareSerial MySerial(1);

void setup() {
    Serial.begin(115200);

    // UART1: RX=36, TX=25
    MySerial.begin(115200, SERIAL_8N1, 34, 33);

    Serial.println("ESP1 Transmitiendo...");
}

void loop() {
    static int counter = 0;

    String mensaje = "Mensaje #" + String(counter++) + "SIUUU VAMOS, ESTO ES COMUNICACION SIII ESOO, NICE";
    MySerial.println(mensaje);

    Serial.println("Enviado: " + mensaje);

    delay(1000);
}