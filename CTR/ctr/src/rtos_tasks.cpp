#include "rtos_tasks.h"
#include "global.h"
#include "serial_com.h"

QueueHandle_t rawSensorQueue;
QueueHandle_t rawSensorTelemetryQueue;

TaskHandle_t getDataTaskHandle = NULL;
TaskHandle_t sendTelemetryTaskHandle = NULL;
TaskHandle_t rwControlTaskHandle = NULL;
TaskHandle_t btHandlingTaskHandle = NULL;

#include "BTS7960.h"
BTS7960 motorController(L_EN_MOTOR, R_EN_MOTOR, L_PWM_MOTOR, R_PWM_MOTOR);


bool enterCtr = false;
bool enableCtr = false;
bool enableDebugBt = true;
float gzForDebug = 0.0f;
float thresholdGz = 1.2f;


void setMotorPWM(int pwm)
{
    pwm = constrain(pwm, -255, 255);

    if (pwm > 0)
    {
        motorController.Enable();
        motorController.TurnRight(pwm);        
    }
    else if (pwm < 0)
    {
        motorController.Enable();
        motorController.TurnLeft(pwm);        
    }
    else
    {
       motorController.Stop();       
    }
}

float computeRWControl(float gz)
{
    float error = -gz;

    uint32_t now = millis();

    float dt = (now - rwController.prev_time) / 1000.0f;

    if (dt <= 0.0f)
        dt = 0.001f;

    float derivative =
        (error - rwController.prev_error) / dt;

    float output =
        rwController.kp * error +
        rwController.kd * derivative;

    rwController.prev_error = error;
    rwController.prev_time = now;

    output = constrain(output, -255.0f, 255.0f);

    return output;
}



void getDataTask(void *pvParameters)
{
    TickType_t lastWakeTime;
    lastWakeTime = xTaskGetTickCount();

    CommsCtrData rxData;
    CommsCtrData telemetryData;

    while (true)
    {        
        if (commsPoll(rxData))
        {
            xQueueOverwrite(rawSensorQueue, &rxData);
            telemetryData = rxData;
            telemetryData.timestamp = millis();
            gzForDebug = rxData.gz;
            xQueueOverwrite(rawSensorTelemetryQueue, &telemetryData);
        }

        vTaskDelayUntil(
            &lastWakeTime,
            pdMS_TO_TICKS(1)
        );
    }
}

void rwControlTask(void *pvParameters)
{
    CommsCtrData sensorData;

    rwController.prev_time = millis();

    TickType_t lastWakeTime = xTaskGetTickCount();

    while (true)
    {
        if (xQueuePeek(rawSensorQueue,
                       &sensorData,
                       0) == pdTRUE)
        {
            float control =
                computeRWControl(sensorData.gz);

            if (enableCtr && fabs(sensorData.gz) > thresholdGz)
            {
                setMotorPWM((int)control);
                enterCtr = true;
            }
            else
            {
                motorController.Stop();
            }

            /* if (sensorData.flight_state == STATE_CONTROL) // && (fabs(sensorData.gz) > 0.5f
            {
                setMotorPWM((int)control);    
                enterCtr = true;
            }else
            {
                setMotorPWM(0);
                enterCtr = false;
            } */

        }

        vTaskDelayUntil(
            &lastWakeTime,
            pdMS_TO_TICKS(10));
    }
}



void sendTelemetryTask(void *pvParameters)
{

    TickType_t lastWakeTime;
    lastWakeTime = xTaskGetTickCount();
    CommsCtrData telemetryData;

    while (true)
    {
        if(xQueueReceive(rawSensorTelemetryQueue, &telemetryData, portMAX_DELAY))
        {            
            Serial.print("Timestamp: ");
            Serial.println(telemetryData.timestamp);
            Serial.print("Altitude: ");
            Serial.println(telemetryData.altitude);
            Serial.print("Vertical Velocity: ");
            Serial.println(telemetryData.vertical_velocity);
            Serial.print("Acceleration (ax, ay, az): ");
            Serial.print(telemetryData.ax);
            Serial.print(", ");
            Serial.print(telemetryData.ay);
            Serial.print(", ");
            Serial.println(telemetryData.az);
            Serial.print("Angular Velocity (gx, gy, gz): ");
            Serial.print(telemetryData.gx);
            Serial.print(", ");
            Serial.print(telemetryData.gy);
            Serial.print(", ");
            Serial.println(telemetryData.gz);
            Serial.print("Quaternion (qw, qx, qy, qz): ");
            Serial.print(telemetryData.qw);
            Serial.print(", ");
            Serial.print(telemetryData.qx);
            Serial.print(", ");
            Serial.print(telemetryData.qy);
            Serial.print(", ");
            Serial.println(telemetryData.qz);
            Serial.print("Flight State: ");
            Serial.print(telemetryData.flight_state);            
            Serial.print("Control Used: ");
            Serial.println(enterCtr ? "Yes" : "No");

        }

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(500));
    }
}

void btHandlingTask(void *pvParameters)
{
    while (true)
    {
        if (SerialBT.available())
        {
            String command = SerialBT.readStringUntil('\n');
            command.trim();

            if (command == "s" || command == "S")
            {
                SerialBT.println("Starting control...");
                enableCtr = true;
            }
            else if (command == "t" || command == "T")
            {
                SerialBT.println("Stopping control...");
                setMotorPWM(0);
                enableCtr = false;
            }else if(command == "B" || command == "b")
            {
                enableDebugBt = !enableDebugBt;
                SerialBT.println(String("Debug BT ") + (enableDebugBt ? "enabled" : "disabled"));
            }else if (command.length() > 1 && (command[0] == 'K' || command[0] == 'k')){
                int valor = command.substring(1).toInt();

                rwController.kp = valor;

                SerialBT.println("Valor actualizado: " + String(rwController.kp));
            }else if (command.length() > 1 && (command[0] == 'D' || command[0] == 'd')){
                int valor = command.substring(1).toInt();

                rwController.kd = valor;

                SerialBT.println("Valor actualizado: " + String(rwController.kd));
            }else if (command.length() > 1 && (command[0] == 'R' || command[0] == 'r')){
                float valor = command.substring(1).toFloat();

                thresholdGz = valor;

                SerialBT.println("Valor actualizado: " + String(thresholdGz));
            }else if (command == "info" || command == "INFO"){
                enableDebugBt = false;
                enableCtr = false;
                SerialBT.println("Comandos: ");
                SerialBT.println("S - Start control");
                SerialBT.println("T - Stop control");
                SerialBT.println("B - Togle debug");
                SerialBT.println("K000 - change Kp to 000 val");
                SerialBT.println("D000 - change Kd to 000 val");
                SerialBT.println("R00 - Change gyro threshold to 000 val");
                
            }else if(command == "l" || command == "L"){
                enableCtr = false;
                enableDebugBt = false;
                
                motorController.Stop();
                SerialBT.println("Control stopped and motor set to 0.");
                SerialBT.println("Starting motor test sequence...");

                delay(2000);
                
                for (int pwm = 0; pwm <= 255; pwm += 51) {
                    SerialBT.println("Turning right with PWM: " + String(pwm));
                    motorController.Enable();
                    motorController.TurnRight(pwm);
                    vTaskDelay(pdMS_TO_TICKS(1000));
                }

                motorController.Stop();
                delay(2000);
                SerialBT.println("Motor stopped. Starting left turn sequence...");

                for (int pwm = 0; pwm <= 255; pwm += 51) {
                    SerialBT.println("Turning left with PWM: " + String(pwm));
                    motorController.Enable();
                    motorController.TurnLeft(pwm);
                    vTaskDelay(pdMS_TO_TICKS(1000));
                }                

                motorController.Stop();
                SerialBT.println("Motor test sequence completed. Motor stopped.");
            }
            
            
            else
            {
                SerialBT.println("Unknown command. Use START or STOP.");
            }
        }

        if (enableDebugBt)
        {
            SerialBT.println("GZ: " + String(gzForDebug));
            SerialBT.println("GZ error: " + String(rwController.prev_error));
            SerialBT.println("GZ dt; " + String(rwController.prev_time));
            SerialBT.println("Control Active: " + String(enterCtr ? "Yes" : "No"));
            SerialBT.println("Threshold GZ: " + String(thresholdGz));
            SerialBT.println("Control enabled: " + String(enableCtr ? "Yes" : "No"));
            SerialBT.println("Kp: " + String(rwController.kp));
            SerialBT.println("Kd: " + String(rwController.kd));
            SerialBT.println("----");
        }    

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}