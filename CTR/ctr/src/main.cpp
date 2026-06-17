#include <Arduino.h>
#include "global.h"
#include "serial_com.h"
#include "rtos_tasks.h"

void setup() {


    pinMode(R_PWM_MOTOR, OUTPUT);
    pinMode(L_PWM_MOTOR, OUTPUT);

    pinMode(R_EN_MOTOR, OUTPUT);
    pinMode(L_EN_MOTOR, OUTPUT);

    digitalWrite(R_EN_MOTOR, HIGH);
    digitalWrite(L_EN_MOTOR, HIGH);

    analogWrite(R_PWM_MOTOR, 0);
    analogWrite(L_PWM_MOTOR, 0);



  Serial.begin(BAUD_RATE);
  delay(2000);
  Serial.println("Starting CTR OBC...");
  Serial1.begin(BAUD_RATE, SERIAL_8N1, CTR_RX, CTR_TX);
  Serial.println("Serial1 initialized");

  SerialBT.begin("Fireblast_Controller_VOLTA");

  rawSensorQueue = xQueueCreate(1, sizeof(CommsCtrData));
  rawSensorTelemetryQueue = xQueueCreate(1, sizeof(CommsCtrData));

  Serial.println("Bt & Queues initialized");

  xTaskCreatePinnedToCore(
    getDataTask,
    "GetDataTask",
    4096,
    NULL,
    5,
    &getDataTaskHandle,
    0
  );

  xTaskCreatePinnedToCore(
    sendTelemetryTask,
    "SendTelemetryTask",
    4096,
    NULL,
    2,
    &sendTelemetryTaskHandle,
    0
  );

  xTaskCreatePinnedToCore(
    rwControlTask,
    "RW Control",
    4096,
    NULL,
    3,
    &rwControlTaskHandle,
    1
);

  xTaskCreatePinnedToCore(
    btHandlingTask,
    "BT Handling",
    4096,
    NULL,
    1,
    &btHandlingTaskHandle,
    0
  );

}

void loop() {}
