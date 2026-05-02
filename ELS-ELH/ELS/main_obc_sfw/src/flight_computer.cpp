#include "flight_computer.h"

#include "constants.h"
#include "sensors_manager.h"    
#include "error_warning.h"      
#include "flash_storage.h"    
#include <Arduino.h>
#include <SD.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

static FlightState gState = STATE_INIT;
static uint32_t gFlashWriteAddr = 0;
static uint8_t gPageBuf[FLASH_PAGE_SIZE];
static uint16_t gPageBufIdx = 0;


static uint16_t crc16(const uint8_t* data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int b = 0; b < 8; b++) {
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : (crc << 1);
        }
    }
    return crc;
}

static void PageBufFlush() {
    if (gPageBufIdx == 0) return;

    if (gFlashWriteAddr + gPageBufIdx > FLASH_TOTAL_BYTES) {
        Serial.println("[DAQ] Flash llena. Deteniendo grabacion.");
        return;
    }

    FlashWrite(gFlashWriteAddr, gPageBuf, gPageBufIdx);
    gFlashWriteAddr += gPageBufIdx;
    gPageBufIdx = 0;
}

static void PageBufWrite(const uint8_t* data, uint16_t len) {
    uint16_t written = 0;
    while (written < len) {
        uint16_t space = FLASH_PAGE_SIZE - gPageBufIdx;
        uint16_t toCopy = (len - written) < space ? (len - written) : space;

        memcpy(gPageBuf + gPageBufIdx, data + written, toCopy);
        gPageBufIdx += toCopy;
        written += toCopy;

        if (gPageBufIdx == FLASH_PAGE_SIZE) {
            PageBufFlush();
        }
    }
}

static void record_fast_packet() {
    FastFlightPacket fast_pkt;
    memset(&fast_pkt, 0, sizeof(FastFlightPacket));
    fast_pkt.packet_id = 0x01;
    fast_pkt.timestamp_ms = millis();
    fast_pkt.mpu = mpuData;
    fast_pkt.bno = bnoData;

    fast_pkt.checksum = crc16((uint8_t*)&fast_pkt, sizeof(FastFlightPacket) - sizeof(uint16_t));

    PageBufWrite((uint8_t*)&fast_pkt, sizeof(FastFlightPacket));
}

static void record_slow_packet() {
    SlowFlightPacket slow_pkt;
    memset(&slow_pkt, 0, sizeof(SlowFlightPacket));
    slow_pkt.packet_id = 0x02;
    slow_pkt.timestamp_ms = millis();
    slow_pkt.bme = bmeData;
    slow_pkt.gps = ubloxData;

    slow_pkt.checksum = crc16((uint8_t*)&slow_pkt, sizeof(SlowFlightPacket) - sizeof(uint16_t));

    PageBufWrite((uint8_t*)&slow_pkt, sizeof(SlowFlightPacket));
}

/*
static void download_flash_to_sd() {

    if (gFlashWriteAddr == 0) {
        Serial.println("[DOWNLOAD] Flash vacia.");
        return;
    }

    File fast_file = SD.open("/fast_log.csv", FILE_WRITE);
    File slow_file = SD.open("/slow_log.csv", FILE_WRITE);

    if (!fast_file) {
        CriticalErrorSensor("No se pudo abrir fast_data.csv en SD");
        return;
    }
    if (!slow_file) {
        CriticalErrorSensor("No se pudo abrir slow_data.csv en SD");
        return;
    }

    fast_file.println(
        "timestamp_ms,"
        "MPU_ax,MPU_ay,MPU_az,MPU_gx,MPU_gy,MPU_gz,MPU_mx,MPU_my,MPU_mz,"
        "BNO_ax,BNO_ay,BNO_az,BNO_gx,BNO_gy,BNO_gz,BNO_mx,BNO_my,BNO_mz,"
        "transducer_voltage,transducer_pressure,"
        "crc_ok"
    );
    slow_file.println(
        "timestamp_ms,"
        "bme_temp,bme_humidity,bme_pressure,bme_altitude,"
        "gps_lat,gps_lon,gps_alt,gps_speed,"
        "crc_ok"
    );

    uint32_t     currentAddr  = 0;
    uint32_t     corrupt_count = 0;

    while (currentAddr < gFlashWriteAddr) {

        uint8_t id;
        FlashRead(currentAddr, &id, 1);

        if (id == 0x01) {
            FastFlightPacket fast_pkt;

            FlashRead(currentAddr, (uint8_t*)&fast_pkt, sizeof(FastFlightPacket));

            uint16_t expected_crc = crc16((uint8_t*)&fast_pkt, sizeof(FastFlightPacket) - sizeof(uint16_t));
            bool crc_ok = (fast_pkt.checksum == expected_crc);
            if (!crc_ok) corrupt_count++;

            char csv_line[256];
            snprintf(csv_line, sizeof(csv_line), 
                "%lu,"
                "%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,"
                "%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,"
                "%.4f,%.4f,"
                "%s",
                (unsigned long)fast_pkt.timestamp_ms,
                fast_pkt.mpu.MPU_ax, fast_pkt.mpu.MPU_ay, fast_pkt.mpu.MPU_az,
                fast_pkt.mpu.MPU_gx, fast_pkt.mpu.MPU_gy, fast_pkt.mpu.MPU_gz,
                fast_pkt.bno.BNO_ax, fast_pkt.bno.BNO_ay, fast_pkt.bno.BNO_az,
                fast_pkt.bno.BNO_gx, fast_pkt.bno.BNO_gy, fast_pkt.bno.BNO_gz,
                fast_pkt.bno.BNO_mx, fast_pkt.bno.BNO_my, fast_pkt.bno.BNO_mz,
                crc_ok ? "OK" : "CORRUPT"
            );
            fast_file.println(csv_line);

            currentAddr += sizeof(FastFlightPacket);
        }

        else if (id == 0x02) {
            SlowFlightPacket slow_pkt;
            FlashRead(currentAddr, (uint8_t*)&slow_pkt, sizeof(SlowFlightPacket));

            uint16_t expected_crc = crc16((uint8_t*)&slow_pkt, sizeof(SlowFlightPacket) - sizeof(uint16_t));
            bool crc_ok = (slow_pkt.checksum == expected_crc);
            if (!crc_ok) corrupt_count++;

            char csv_line[192];
            snprintf(csv_line, sizeof(csv_line), 
                "%lu,%.2f,%.2f,%.2f,%.2f,%.6f,%.6f,%.2f,%.2f,%s",
                (unsigned long)slow_pkt.timestamp_ms,
                slow_pkt.bme.temp,
                slow_pkt.bme.humidity,
                slow_pkt.bme.pressure,
                slow_pkt.bme.altitude,
                slow_pkt.gps.latitude,
                slow_pkt.gps.longitude,
                slow_pkt.gps.altitude,
                slow_pkt.gps.speed,
                crc_ok ? "OK" : "CORRUPT"
            );    
            slow_file.println(csv_line);

            currentAddr += sizeof(SlowFlightPacket);
        }

        else {
            currentAddr++;
            corrupt_count++;
        }

    }

    fast_file.close();
    slow_file.close();

    Serial.printf("[DOWNLOAD] Listo. Bytes leidos: %lu. Errores: %lu\n",
        (unsigned long)gFlashWriteAddr,
        (unsigned long)corrupt_count
    );

    digitalWrite(LED_RED_PIN, LOW);
    digitalWrite(LED_BLUE_PIN, LOW);
    digitalWrite(LED_GREEN_PIN, HIGH);
}
*/

void VerifyFlashContent() {
    Serial.println("\n--- INICIANDO VERIFICACIÓN DE FLASH ---");
    
    if (gFlashWriteAddr == 0) {
        Serial.println("[!] La Flash está vacía (puntero en 0).");
        return;
    }

    Serial.printf("[INFO] Datos totales escritos: %lu bytes\n", gFlashWriteAddr);
    Serial.println("----------------------------------------");

    uint32_t currentAddr = 0;
    uint32_t countFast = 0;
    uint32_t countSlow = 0;
    uint32_t countCorrupt = 0;

    while (currentAddr < gFlashWriteAddr) {
        uint8_t id = 0;
        // Leemos solo el primer byte para identificar el tipo de paquete
        FlashRead(currentAddr, &id, 1);

        if (id == 0x01) { // Paquete Rápido (MPU + BNO)
            FastFlightPacket p;
            FlashRead(currentAddr, (uint8_t*)&p, sizeof(FastFlightPacket));
            
            // Verificación de CRC
            uint16_t crc_calc = crc16((uint8_t*)&p, sizeof(FastFlightPacket) - 2);
            bool ok = (p.checksum == crc_calc);

            Serial.printf("[FAST] Addr: 0x%06lX | TS: %lu | AccelX: %.2f | CRC: %s\n", 
                          currentAddr, p.timestamp_ms, p.mpu.MPU_ax, ok ? "OK" : "ERROR");
            
            if (!ok) countCorrupt++;
            countFast++;
            currentAddr += sizeof(FastFlightPacket);
        } 
        else if (id == 0x02) { // Paquete Lento (BME + GPS)
            SlowFlightPacket p;
            FlashRead(currentAddr, (uint8_t*)&p, sizeof(SlowFlightPacket));
            
            uint16_t crc_calc = crc16((uint8_t*)&p, sizeof(SlowFlightPacket) - 2);
            bool ok = (p.checksum == crc_calc);

            Serial.printf("[SLOW] Addr: 0x%06lX | TS: %lu | Pres: %.2f | GPS_Lat: %.6f | CRC: %s\n", 
                          currentAddr, p.timestamp_ms, p.bme.pressure, p.gps.latitude, ok ? "OK" : "ERROR");

            if (!ok) countCorrupt++;
            countSlow++;
            currentAddr += sizeof(SlowFlightPacket);
        } 
        else {
            // Si encontramos algo que no es 0x01 ni 0x02, es ruido o error de alineación
            Serial.printf("[?] Byte desconocido en 0x%06lX: 0x%02X (Saltando...)\n", currentAddr, id);
            currentAddr++; 
            countCorrupt++;
        }

        // Pequeña pausa para no saturar el buffer del Serial si hay miles de datos
        if ((countFast + countSlow) % 50 == 0) delay(5);
    }

    Serial.println("----------------------------------------");
    Serial.println("RESUMEN DE VERIFICACIÓN:");
    Serial.printf("  - Paquetes Fast: %lu\n", countFast);
    Serial.printf("  - Paquetes Slow: %lu\n", countSlow);
    Serial.printf("  - Errores/Bytes corruptos: %lu\n", countCorrupt);
    Serial.println("--- FIN DE VERIFICACIÓN ---\n");
}

void flight_computer_init() {

    if (!FlashInit()) {
        CriticalErrorSensor("W25Q128 no detectada. Verificar HSPI y FLASH_CS.");
    }
    else{
    Serial.println("[DAQ] Flash W25Q128 OK.");
    }
    if (!SD.begin(SD_CS)) {
        Serial.println("[DAQ] ADVERTENCIA: SD no detectada.");
    } 
    else {
        Serial.println("[DAQ] SD OK.");
    }

    Serial.println("[DAQ] Borrando flash (esperar)...");
    FlashEraseChip();
    Serial.println("[DAQ] Flash borrada OK.");

    gFlashWriteAddr = 0;
    gPageBufIdx = 0;

    Serial.printf("[DAQ] Flash: %lu MB disponibles.\n", 
        FLASH_TOTAL_BYTES / (1024UL * 1024UL));
    Serial.printf("[DAQ] Estimacion: ~%.1f min de vuelo (FastPackets a 100Hz)\n", 
        (float)(FLASH_TOTAL_BYTES / sizeof(FastFlightPacket)) / 6000.0f);

    digitalWrite(LED_BLUE_PIN, HIGH);
    gState = STATE_IDLE; //Para prueba (cambiar a STATE_PAD para vuelo)
    Serial.println("[DAQ] MODO PRUEBA: grabando en STATE_ASCENT. Presionar boton para descargar.");
}

void flight_computer_update() {

    // camelCase
    static uint32_t lastFastSample = 0;
    static uint32_t lastSlowSample = 0;
    static uint32_t accelStartMs   = 0; 
    static float    maxAltitude     = -999.0f;
    static uint8_t  apogeeCount     = 0;
    static uint32_t stableStartMs  = 0;
    static uint32_t landedStartMs  = 0;
    static float    lastLandedAlt  = 0.0f;

    uint32_t now = millis();

    switch (gState) {

    case STATE_IDLE: 

        digitalWrite(LED_BLUE_PIN, HIGH);

        if (now - lastSlowSample >= 1000) {
            lastSlowSample = now;
            ReadBME280();
            ReadUblox();
            ReadBNO055();
/*
            Serial.printf("[IDLE] OK, Actuators inhibided, BME280 T: %.2fC P: %.2fPa A: %.2fm, Ublox Lat: %.6f Lon: %.6f Alt: %.2fm Spd: %.2fm/s, MPU9250 Accel: (%.2f, %.2f, %.2f) m/s², BNO055 Accel: (%.2f, %.2f, %.2f) m/s²\n",
                bmeData.temp, bmeData.pressure, bmeData.altitude,
                ubloxData.latitude, ubloxData.longitude, ubloxData.altitude, ubloxData.speed,
                mpuData.MPU_ax, mpuData.MPU_ay, mpuData.MPU_az,
                bnoData.BNO_ax, bnoData.BNO_ay, bnoData.BNO_az
            );
*/
            Serial.print("Escribe un comando: ");

            if (Serial.available() > 0) {
                String cmd = Serial.readStringUntil('\n');
                cmd.trim();
                if (cmd == "INTEGRATION") {
                    Serial.println("[COMANDO] Pasando a INTEGRATION.");
                    gState = STATE_INTEGRATION;
                    digitalWrite(LED_BLUE_PIN, LOW);
                    digitalWrite(LED_GREEN_PIN, HIGH);
                }
                else if (cmd == "SAVE SLOW DATA") {
                    Serial.println("[COMANDO] Guardando un paquete lento de prueba.");
                    record_slow_packet();
                }
                else if (cmd == "SAVE FAST DATA") {
                    Serial.println("[COMANDO] Guardando un paquete rapido de prueba.");
                    record_fast_packet();
                }
                else if (cmd == "DOWNLOAD") {
                    Serial.println("[COMANDO] Iniciando descarga por comando serial.");
                    //// download_flash_to_sd();
                }
                else if (cmd == "ERASE") {
                    Serial.println("[COMANDO] Borrando flash por comando serial.");
                    FlashEraseChip();
                    gFlashWriteAddr = 0;
                    gPageBufIdx = 0;
                }
                if (cmd == "PAD") {
                    Serial.println("[COMANDO] Pasando a PAD.");
                    gState = STATE_PAD;
                    digitalWrite(LED_BLUE_PIN, LOW);
                    digitalWrite(LED_RED_PIN, HIGH);
                }
            }

            
        }
        
        break;

    case STATE_INIT:
        //arranque del sistema
        break;

    case STATE_INTEGRATION:
        if (now - lastSlowSample >= 5000) {
            lastSlowSample = now;
            ReadBME280();
            record_slow_packet();
        }
        break;

    case STATE_PAD:

        if (now - lastSlowSample >= 10000) {
            lastSlowSample = now;

            ReadBME280();
            ReadUblox();
            record_slow_packet();
        }
        if (now - lastFastSample >= FAST_SAMPLE_INTERVAL_MS) { 
            lastFastSample = now;
            ReadBME280(); 

            float total_accel = sqrtf(
                mpuData.MPU_ax * mpuData.MPU_ax +
                mpuData.MPU_ay * mpuData.MPU_ay +
                mpuData.MPU_az * mpuData.MPU_az
            );

            if (total_accel > LAUNCH_ACCEL_THRESHOLD_MS2) {
                if (accelStartMs == 0) accelStartMs = now;
                
                if ((now - accelStartMs >= 500) && (bmeData.altitude > 30.0f)) {
                    Serial.println("[EVENTO] DESPEGUE DETECTADO. Pasando a ASCENT.");
                    gState = STATE_ASCENT;
                    digitalWrite(LED_RED_PIN, LOW);
                    digitalWrite(LED_GREEN_PIN, HIGH);
                }
            } 
            else {
                accelStartMs = 0;
            }
        }
        break;

    case STATE_ASCENT:
        if (now - lastFastSample >= FAST_SAMPLE_INTERVAL_MS) {
            lastFastSample = now;
            ReadBNO055();
            record_fast_packet(); // 100 Hz
        }
        if (now - lastSlowSample >= SLOW_SAMPLE_INTERVAL_MS) {
            lastSlowSample = now;
            ReadBME280();
            ReadUblox();
            record_slow_packet(); // 1 Hz

            if (bmeData.altitude > maxAltitude) {
                maxAltitude = bmeData.altitude;
                apogeeCount = 0;
            } 
            else if (bmeData.altitude < (maxAltitude - 5.0f)) {
                apogeeCount++;
                if (apogeeCount >= 10) {
                    Serial.println("[EVENTO] APOGEO DETECTADO. Pasando a EJECTION.");
                    gState = STATE_EYECTION;
                    digitalWrite(LED_GREEN_PIN, LOW);
                }
            } 
            else {
                apogeeCount = 0;
            }
        }
        break;

    case STATE_EYECTION:
        if (now - lastFastSample >= FAST_SAMPLE_INTERVAL_MS) {
            lastFastSample = now;
            ReadBNO055();
            record_fast_packet();

            float total_gyro = sqrtf(
                mpuData.MPU_gx * mpuData.MPU_gx +
                mpuData.MPU_gy * mpuData.MPU_gy +
                mpuData.MPU_gz * mpuData.MPU_gz
            );
            
            // Falta ademas comprobar que los kill switches dejaron de estar presionados
            if (total_gyro < 20.0f) {  // por que 20, cuando se estabilize no podria ser mayor?
                if (stableStartMs == 0) stableStartMs = now;
                if (now - stableStartMs > 2000) {
                    Serial.println("[EVENTO] Cubesat estabilizado. Pasando a CONTROL.");
                    gState = STATE_CONTROL;
                }
            } 
            else {
                stableStartMs = 0;
            }
        }
        if (now - lastSlowSample >= SLOW_SAMPLE_INTERVAL_MS) {
            lastSlowSample = now;
            ReadBME280();
            ReadUblox();
            record_slow_packet();
        }
        break;

    case STATE_CONTROL:

        if (now - lastFastSample >= FAST_SAMPLE_INTERVAL_MS) {
            lastFastSample = now;
            ReadBNO055();
            record_fast_packet();
        }
        if (now - lastSlowSample >= SLOW_SAMPLE_INTERVAL_MS) {
            lastSlowSample = now;
            ReadBME280();
            ReadUblox();
            record_slow_packet();

            if (bmeData.altitude < 500.0f) {
                Serial.println("[EVENTO] Umbral de drenaje cruzado. Pasando a DRAIN.");
                gState = STATE_DRAIN;
            }
        }
        break;

    case STATE_DRAIN:
        if (now - lastFastSample >= FAST_SAMPLE_INTERVAL_MS) {
            lastFastSample = now;
            ReadBNO055();
            record_fast_packet();
        }
        if (now - lastSlowSample >= SLOW_SAMPLE_INTERVAL_MS) {
            lastSlowSample = now;
            ReadBME280();
            ReadUblox();
            record_slow_packet();

            if (fabsf(bmeData.altitude - lastLandedAlt) < 2.0f) {
                if (landedStartMs == 0) landedStartMs = now;
                if (now - landedStartMs > 5000) { 

                    PageBufFlush(); 
                    Serial.println("[EVENTO] ATERRIZAJE CONFIRMADO. Pasando a RECOVERY.");
                    gState = STATE_RECOVERY;
                    digitalWrite(LED_RED_PIN, LOW);
                }
            }
            else {
                lastLandedAlt = bmeData.altitude;
                landedStartMs = 0;
            }
        }
        break;

    case STATE_RECOVERY:

        digitalWrite(LED_GREEN_PIN, (now % 1000) < 500 ? HIGH : LOW);

        if (now - lastSlowSample >= 10000) {
            lastSlowSample = now;
            ReadBME280(); 
        }
        break;

    case STATE_DOWNLOAD:

        // download_flash_to_sd();
        Serial.println("[DAQ] Descarga completa. Reiniciar para nuevo vuelo.");
        while (true) {
            digitalWrite(LED_GREEN_PIN, HIGH);
            delay(1000);
        }
        break;

    default:
        break;
    }
}

FlightState FlightComputerGetState() {
    return gState;
}