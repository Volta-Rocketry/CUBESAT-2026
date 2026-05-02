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
#include "linear_kalman_nav.h"

KalmanFilterNav NavFilter;

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
        println("Flash Full, Stopping recording");
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

static void RecordFastPacket() {
    FastFlightPacket fast_pkt;
    memset(&fast_pkt, 0, sizeof(FastFlightPacket));
    fast_pkt.packet_id = 0x01;
    fast_pkt.timestamp_ms = millis();
    fast_pkt.mpu = mpuData;
    fast_pkt.bno = bnoData;

    fast_pkt.checksum = crc16((uint8_t*)&fast_pkt, sizeof(FastFlightPacket) - sizeof(uint16_t));

    PageBufWrite((uint8_t*)&fast_pkt, sizeof(FastFlightPacket));
}

static void RecordSlowPacket() {
    SlowFlightPacket slow_pkt;
    memset(&slow_pkt, 0, sizeof(SlowFlightPacket));
    slow_pkt.packet_id = 0x02;
    slow_pkt.timestamp_ms = millis();
    slow_pkt.bme = bmeData;
    slow_pkt.gps = ubloxData;

    slow_pkt.checksum = crc16((uint8_t*)&slow_pkt, sizeof(SlowFlightPacket) - sizeof(uint16_t));

    PageBufWrite((uint8_t*)&slow_pkt, sizeof(SlowFlightPacket));
}

void VerifyFlashContent() {
    println("Starting FLASH Verification");
    
    if (gFlashWriteAddr == 0) {
        println("FLASH is empty");
        return;
    }

    println("Total written data: %lu bytes\n", gFlashWriteAddr);

    uint32_t currentAddr = 0;
    uint32_t countFast = 0;
    uint32_t countSlow = 0;
    uint32_t countCorrupt = 0;

    while (currentAddr < gFlashWriteAddr) {
        uint8_t id = 0;

        FlashRead(currentAddr, &id, 1);

        if (id == 0x01) {
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
        else if (id == 0x02) {
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
            Serial.printf("[?] Byte desconocido en 0x%06lX: 0x%02X (Saltando...)\n", currentAddr, id);
            currentAddr++; 
            countCorrupt++;
        }

        if ((countFast + countSlow) % 50 == 0) delay(5);
    }

    println("Verification results");
    Serial.printf("  - Fast Packets: %lu\n", countFast);  // Puedo usar println para estos?
    Serial.printf("  - Slow Packets: %lu\n", countSlow);
    Serial.printf("  - Corrupted Bytes: %lu\n", countCorrupt);
    println("Verification end");
}

void flight_computer_init() {

    FlashInit();

    println("Erasing FLASH...");
    FlashEraseChip();
    println("FLASH erased");

    gFlashWriteAddr = 0;
    gPageBufIdx = 0;

    Serial.printf("Flash: %lu MB avaiable.\n", 
        FLASH_TOTAL_BYTES / (1024UL * 1024UL));
    Serial.printf("Estimated: ~%.1f flight min (FastPackets 100Hz)\n", 
        (float)(FLASH_TOTAL_BYTES / sizeof(FastFlightPacket)) / 6000.0f);

    digitalWrite(LED_BLUE_PIN, HIGH);
    gState = STATE_IDLE; //Para prueba (cambiar a STATE_PAD para vuelo)
    println("TEST MODE");
}

void flight_computer_update() {

    static uint32_t lastFastSample = 0;
    static uint32_t lastSlowSample = 0;
    static uint32_t accelStartMs   = 0; 
    static float    maxAltitude     = -999.0f;
    static uint8_t  apogeeCount     = 0;
    static uint32_t stableStartMs  = 0;
    static uint32_t landedStartMs  = 0;
    static float    lastLandedAlt  = 0.0f;

    uint32_t now = millis();

//---------- Verify implementation - Where to implement the dt time
    ReadMPU6050();
    ReadBME280();

    float accelZ = mpuData.MPU_az - 9.81f; 
    filtroNav.predict(accelZ, dt);

    //implement an if function for new baro data
    float altitud_baro = bmeData.altitude;
    filtroNav.update(altitud_baro);
    
    float filtered_altitude = filtroNav.getAltitude();
    float filtered_velocity = filtroNav.getVelocity();

    if (vel_actual > 280.0f) {
        kfNav.setRBaro(1000.0f); 
    } 
    else {
        kfNav.setRBaro(2.0f); 
    }
//-----------

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
            println("Write a command: ");

            if (Serial.available() > 0) {
                String cmd = Serial.readStringUntil('\n');
                cmd.trim();
                if (cmd == "INTEGRATION") {
                    println("Transition to INTEGRATION");
                    gState = STATE_INTEGRATION;
                    digitalWrite(LED_BLUE_PIN, LOW);
                    digitalWrite(LED_GREEN_PIN, HIGH);
                }
                else if (cmd == "SAVE SLOW DATA") {
                    Serial.println("Saving SLOW packet");
                    record_slow_packet();
                }
                else if (cmd == "SAVE FAST DATA") {
                    Serial.println("Saving FAST packet");
                    record_fast_packet();
                }
                else if (cmd == "DOWNLOAD") {
                    Serial.println("Downloading data to FLASH");
                    //// download_flash_to_sd();
                }
                else if (cmd == "ERASE") {
                    Serial.println("Erasing FLASH");
                    FlashEraseChip();
                    gFlashWriteAddr = 0;
                    gPageBufIdx = 0;
                }
                if (cmd == "PAD") {
                    Serial.println("Transition to PAD");
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
                    println("Boost detected. Transition to ASCENT.");
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
                    Serial.println("Apogee detected. transition to EJECTION.");
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
                    println("Control safety conditions. Transition to CONTROL.");
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
                println("Drain threshold. Transition to DRAIN");
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
                    println("LANDING CONFIRMED. Transition to RECOVERY.");
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

    default:
        break;
    }
}

FlightState FlightComputerGetState() {
    return gState;
}