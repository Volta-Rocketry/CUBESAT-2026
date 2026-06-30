#include "signals.h"

#include <Arduino.h>
#include "constants.h"
#include "BluetoothSerial.h"

extern BluetoothSerial SerialBT;

void initPCB() {
    uint8_t blinkCount = 0;
    while (blinkCount < 3) {
        colorRGB(0, 0, 255);
        delay(BLINK);
        colorRGB(0, 0, 0);
        delay(BLINK);
        blinkCount++;
    }
}

void println(const char* messages){
    Serial.print("[MESSAGE]: ");
    Serial.println(messages);
    SerialBT.print("[MESSAGE]: ");
    SerialBT.println(messages);
}

void colorRGB(int r, int g, int b) {
  ledcWrite(RED_CHANNEL, constrain(r, 0, 255));
  ledcWrite(GREEN_CHANNEL, constrain(g, 0, 255));
  ledcWrite(BLUE_CHANNEL, constrain(b, 0, 255));
}

void PWMBuzzer(unsigned int f, unsigned long d) {
    static unsigned long initBuzzer;
    static bool buzzerState = false;

    if (!buzzerState) { 
        ledcWriteTone(BUZZER_CHANNEL, f);
        initBuzzer = millis();
        buzzerState = true;
    }
    if (buzzerState) {
        if (millis() - initBuzzer >= d) {
        ledcWriteTone(BUZZER_CHANNEL, 0);
        buzzerState = false;
        }
    }
}