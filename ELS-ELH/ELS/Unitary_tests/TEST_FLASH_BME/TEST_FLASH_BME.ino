#include <Arduino.h>
#include <SPI.h>
#include "flash_storage.h"

bool result = false;

//datos prueba

uint32_t direccion_test = 0x000100;
const char mensaje_original[] = "TEXTO DE PRUEBA 22/04/2026";
const uint16_t longitud = sizeof(mensaje_original);
uint8_t buffer_lectura[sizeof(mensaje_original)];

void setup() {

    Serial.begin(115200);
    hspi.begin(18, 19, 23, -1);
    delay(100);

    pinMode(PIN_CS_FLASH, OUTPUT);
    digitalWrite(PIN_CS_FLASH, HIGH);
    pinMode(PIN_CS_BME, OUTPUT);
    digitalWrite(PIN_CS_BME, HIGH);

    delay(2000);

    Serial.println("-------------------INICIALIZACIÓN FLASH------------------- ");
    result = flash_init();
    if(result) {
        Serial.println("[FLASH OK] Flash inicializada");
    }
    else {
        Serial.print("[FLASH ERROR] Flash no inicializada");
    }

    InitBME280();

    Serial.println();
    Serial.println("------------------------------------------");
    Serial.println("Comandos disponibles:");
    Serial.println("ERASE — Borra el chip completo");
    Serial.println("WRITE — Escribe mensaje de prueba");
    Serial.println("READ  — Lee y verifica el mensaje");
    Serial.println("BME   — Lee y muestra datos del BME280");
    Serial.println("------------------------------------------");

}

void loop() {

        if (Serial.available() >0 ){

            String command = Serial.readStringUntil('\n');
            command.trim();
            Serial.println(command);

            if (command == "ERASE") {
                if (result) {
                    Serial.println("-------- BORRADO FLASH -------- ");
                    Serial.print("[FLASH OK] Borrando chip completo");
                    flash_erase_chip();
                    Serial.println("\n[FLASH OK] Borrado completado");
                }
                else {
                    Serial.println("[FLASH ERROR] No se inicializó la flash");
                }
            }

            else if (command == "WRITE"){
                if (result) {
                    Serial.println("-------- ESCRIBIENDO FLASH -------- ");
                    Serial.printf("[FLASH OK] Escribiendo en 0x%06X: '%s'\n", direccion_test, mensaje_original);
                    flash_write(direccion_test, (uint8_t*)mensaje_original, longitud);
                    Serial.println("[FLASH OK] Escritura terminada");
                }
                else {
                    Serial.println("[FLASH ERROR] No se inicializó la flash");
                }
            }
            else if (command == "READ"){
                if (result) {
                    Serial.println("-------------------LECTURA FLASH------------------- ");
                    Serial.println("[FLASH OK] Leyendo datos para verificar...");
                    flash_read(direccion_test, buffer_lectura, longitud);
                    Serial.printf("[FLASH OK] Dato leído: '%s'\n", (char*)buffer_lectura);

                    if (strcmp(mensaje_original, (char*)buffer_lectura) == 0) {
                        Serial.println("[FLASH OK] Los datos coinciden");
                    } 
                    else {
                        Serial.println("[FLASH ALERT] Los datos NO coinciden");
                    }
                }
                else {
                    Serial.println("[FLASH ERROR] No se inicializó la flash");
                }

            }
            else if (command == "BME"){
                Serial.println("-------------------LECTURA BME------------------- ");
                Serial.println("[BME OK] Leyendo datos para verificar...");
                ReadBME280();

                Serial.printf("[BME] Timestamp:   %lu ms\n",  bmeData.timestamp);
                Serial.printf("[BME] Temperatura: %.2f °C\n", bmeData.temp);
                Serial.printf("[BME] Humedad:     %.2f %%\n",  bmeData.humidity);
                Serial.printf("[BME] Presion:     %.2f Pa\n",  bmeData.pressure);
                Serial.printf("[BME] Altitud:     %.2f m\n",   bmeData.altitude);
            }
            
        }

}