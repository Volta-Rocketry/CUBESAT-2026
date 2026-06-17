#pragma once
#include <Arduino.h>

void getDataTask(void *pvParameters);
void sendTelemetryTask(void *pvParameters);
void rwControlTask(void *pvParameters);
void btHandlingTask(void *pvParameters);

extern TaskHandle_t getDataTaskHandle;
extern TaskHandle_t sendTelemetryTaskHandle;
extern TaskHandle_t rwControlTaskHandle;
extern TaskHandle_t btHandlingTaskHandle;

extern QueueHandle_t rawSensorQueue;
extern QueueHandle_t rawSensorTelemetryQueue;