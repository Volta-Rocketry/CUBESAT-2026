#include "flight_computer.h"
#include "comm_manager.h"
#include "constants.h"
#include "sensors_manager.h"    
#include "error_warning.h"      
#include "signals.h"
#include "flash_storage.h"    
#include "data_processing.h"
#include "madgwick_filter.h"
#include <Arduino.h>
#include <SD.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

CommsInitData dataToInit;
StructInitCom initCom;

uint32_t gFlashWriteAddr = 0;
static FlightState gState = STATE_INIT;
static uint8_t gPageBuf[FLASH_PAGE_SIZE];
static uint16_t gPageBufIdx = 0;
static float beta = 0.3;

/**
 * @brief Page Buffer Execution.
 * * Send the data to be written by flash when the 
 * buffer is already the size of a flash page.
 */
static void pageBufFlush() {
    if (gPageBufIdx == 0) return;

    if (gFlashWriteAddr + gPageBufIdx > FLASH_TOTAL_BYTES) {
        println("Flash Full, Stopping recording");
        return;
    }

    flashWrite(gFlashWriteAddr, gPageBuf, gPageBufIdx);
    gFlashWriteAddr += gPageBufIdx;
    gPageBufIdx = 0;
}

/**
 * @brief Page Buffer Organization.
 * * It organizes the data in the buffer until 
 * it reaches the size of the Flash page.
 */
static void pageBufWrite(const uint8_t* data, uint16_t len) {
    uint16_t written = 0;
    while (written < len) {
        uint16_t space = FLASH_PAGE_SIZE - gPageBufIdx;
        uint16_t toCopy = (len - written) < space ? (len - written) : space;

        memcpy(gPageBuf + gPageBufIdx, data + written, toCopy);
        gPageBufIdx += toCopy;
        written += toCopy;

        if (gPageBufIdx == FLASH_PAGE_SIZE) {
            pageBufFlush();
        }
    }
}

/**
 * @brief Saves the Fast Packet according the data structure.
 * * It obtains the data from the sensors and stores it 
 * according to the structure, for sending to the buffer.
 */
void recordFastPacket() {
    FastFlightPacket fast_pkt;
    memset(&fast_pkt, 0, sizeof(FastFlightPacket));
    fast_pkt.packet_id = 0x01;
    fast_pkt.timestamp_ms = millis();
    fast_pkt.mpu = mpuData;
    fast_pkt.qmc = qmcData;
    fast_pkt.madgwick = madgwickState;
    fast_pkt.bno = bnoData;
    fast_pkt.filter = altitudeFilter;

    fast_pkt.checksum = crc16CCITT((uint8_t*)&fast_pkt, sizeof(FastFlightPacket) - sizeof(uint16_t));

    pageBufWrite((uint8_t*)&fast_pkt, sizeof(FastFlightPacket));
}

/**
 * @brief Saves the Slow Packet according the data structure.
 * * It obtains the data from the sensors and stores it 
 * according to the structure, for sending to the buffer.
 */
void recordSlowPacket() {
    SlowFlightPacket slow_pkt;
    memset(&slow_pkt, 0, sizeof(SlowFlightPacket));
    slow_pkt.packet_id = 0x02;
    slow_pkt.timestamp_ms = millis();
    slow_pkt.bme = bmeData;
    slow_pkt.bmp = bmpData;
    slow_pkt.gps = ubloxData;

    slow_pkt.checksum = crc16CCITT((uint8_t*)&slow_pkt, sizeof(SlowFlightPacket) - sizeof(uint16_t));

    pageBufWrite((uint8_t*)&slow_pkt, sizeof(SlowFlightPacket));
}

/**
 * @brief Reads one CTR frame from Serial and fills sensor globals (SIL mode only).
 * Non-blocking: drains whatever bytes are available on this call.
 */
void silReadSerial() {
    static uint8_t buf[CTR_TP_FRAME_SIZE];
    static uint8_t idx = 0;

    while (Serial.available() > 0) {
        uint8_t c = (uint8_t)Serial.read();

        if (idx == 0 && c != COMM_SYNC_1) continue;
        if (idx == 1 && c != COMM_SYNC_2) { idx = 0; continue; }

        buf[idx++] = c;

        if (idx >= CTR_TP_FRAME_SIZE) {
            idx = 0;

            if (buf[2] != ID_CTR_TP) continue;

            uint16_t crc_rx   = (uint16_t)buf[CTR_TP_FRAME_SIZE - 2] |
                                ((uint16_t)buf[CTR_TP_FRAME_SIZE - 1] << 8);
            uint16_t crc_calc = crc16CCITT(buf, CTR_TP_FRAME_SIZE - 2);
            if (crc_rx != crc_calc) continue;

            const uint8_t* p = buf + 4;
            uint32_t ts;
            float alt, vz, ax, ay, az, gx, gy, gz, qw, qx, qy, qz;

            memcpy(&ts,  p,      4);
            memcpy(&alt, p + 4,  4);
            memcpy(&vz,  p + 8,  4);
            memcpy(&ax,  p + 12, 4);
            memcpy(&ay,  p + 16, 4);
            memcpy(&az,  p + 20, 4);
            memcpy(&gx,  p + 24, 4);
            memcpy(&gy,  p + 28, 4);
            memcpy(&gz,  p + 32, 4);
            memcpy(&qw,  p + 36, 4);
            memcpy(&qx,  p + 40, 4);
            memcpy(&qy,  p + 44, 4);
            memcpy(&qz,  p + 48, 4);

            bnoData.timestamp     = ts;
            bnoData.BNO_ax        = ax;  bnoData.BNO_ay = ay;  bnoData.BNO_az = az;
            bnoData.BNO_gx        = gx;  bnoData.BNO_gy = gy;  bnoData.BNO_gz = gz;
            bnoData.BNO_qw        = qw;  bnoData.BNO_qx = qx;
            bnoData.BNO_qy        = qy;  bnoData.BNO_qz = qz;
            bnoData.BNO_global_az = az;

            bmeData.altitude = alt;

            altitudeFilter.filteredAltitude = alt;
            altitudeFilter.verticalVelocity = vz;
            altitudeFilter.verticalAccel    = az;
        }
    }
}

/**
 * @brief Initializes the flight computer.
 * * Verifies flash space avaiable and initial flight state.
 */
void flightComputerInit() {

    if (sil_command) {
        println("SIL MODE ACTIVE - sensor init skipped");
        flashInit();
        gFlashWriteAddr = 0;
        gPageBufIdx     = 0;
        madgwickInit(&madgwickState, beta);
        altitudeFilter.filteredAltitude = 0.0f;
        altitudeFilter.verticalVelocity = 0.0f;
        altitudeFilter.verticalAccel    = 0.0f;
        altitudeFilter.alpha            = 0.9f;
        gState = STATE_PAD;
        println("SIL PAD MODE");
        return;
    }

    uint32_t time1= millis();

    while (!initCom.comControl && !initCom.comCamera) {

        if (!initCom.comControl) {
            memset(&dataToInit, 0, sizeof(CommsInitData));
            dataToInit.id_to_init = ID_CTR_TP;
            bool ctrOk = commsInit(Serial1, CTR_RX, CTR_TX, &dataToInit);
            
            if (ctrOk) {
                println("CTR Communication initialization completed");
                initCom.comControl = 1;
            }
            else {
                criticalErrorSensor("CTR Communication initialization failed");
                initCom.comControl = 0;
            }
        }

        if (!initCom.comCamera) {
            memset(&dataToInit, 0, sizeof(CommsInitData)); 
            dataToInit.id_to_init = ID_CTR_TP;
            bool camOk = commsInit(Serial2, CAM_RX, CAM_TX, &dataToInit);

            if (camOk) {
                println("CAM Communication initialization completed");
                initCom.comCamera = 1;
            }
            else {
                criticalErrorSensor("CAM Communication initialization failed");
                initCom.comCamera = 0;
            }
        }

        if (millis() - time1 >= 5000) {
            criticalErrorSensor("Communication initialization failed");
            break;
        }
    }

    uint32_t time2= millis();

    flashInit();
    gFlashWriteAddr = 0;
    gPageBufIdx = 0;

    while (!initSensor.initBNO && !initSensor.initMPU) {
        initMPU6050();
        initBMP180();
        initQMC5883L();
        initBNO055();
        initBME280();
        initUblox();
        delay(1);
        if (millis() - time2 >= 5000) {
            criticalErrorSensor("Sensor initialization failed");
            break;
        }
    }

    uint32_t time3= millis();
    while (!calibSensor.calibBNO && !calibSensor.calibMPU && !calibSensor.calibBMP && !calibSensor.calibBME) {
        calibrateSensors();
        delay(1);
        if (millis() - time3 >= 5000) {
            criticalErrorSensor("Sensor calibration failed");
            break;
        }
    }

    if (initSensor.initFlash && initCom.comControl && initCom.comCamera &&
        initSensor.initMPU && initSensor.initBMP && initSensor.initQMC &&
        initSensor.initBNO && initSensor.initBME && initSensor.initGPS) {
        println("All systems initialized successfully");
    } else {
        criticalErrorSensor("Initialization failed for one or more components");
    }
    
    madgwickInit(&madgwickState, beta);

    altitudeFilter.filteredAltitude = 0.0f;
    altitudeFilter.verticalVelocity = 0.0f;    
    altitudeFilter.verticalAccel = 0.0f;
    altitudeFilter.alpha = 0.9f;

    gState = STATE_IDLE;
    println("PAD MODE");
}

/**
 * @brief Updates the flight computer.
 * * Determines functions and actions in each phase of flight, 
 * in addition to establishing the conditions for phase transition.
 */
void flightComputerUpdate() {

    static uint32_t lastFastSample = 0;
    static uint32_t lastSlowSample = 0;
    static uint32_t accelStartMs   = 0; 
    static uint32_t altitudeStartMs   = 0; 
    static float    maxAltitude     = -999.0f;
    static uint8_t  apogeeCount     = 0;
    static uint32_t stableStartMs  = 0;
    static uint32_t drainStartMs   = 0;
    static uint32_t landedStartMs  = 0;
    static float    lastLandedAlt  = 0.0f;
    static float    totalAccel      = 0.0f;
    static uint32_t lastSilPrintMs  = 0;

    unsigned long now = millis();

    switch (gState) {

    case STATE_IDLE: {
        colorRGB(0, 0, 0);
        colorRGB(0, 0, 255);

        if (Serial.available() > 0) {
            String cmd = Serial.readStringUntil('\n');
            cmd.trim();

            if (cmd == "SLOW DATA") {
                Serial.println("Saving SLOW packet");
                processSlowSensors();
            }
            else if (cmd == "FAST DATA") {
                Serial.println("Saving FAST packet");
                processFastSensors();
            }
            else if (cmd == "ERASE") {
                Serial.println("Erasing FLASH");
                flashEraseChip();
                gFlashWriteAddr = 0;
                gPageBufIdx = 0;
            }
            else if (cmd == "DOWNLOAD") {
                Serial.println("Downloading FLASH content");
                pageBufFlush();
                verifyFlashContent();
            }
            else if (cmd == "SEND CTR") {
                Serial.println("Sending CTR package");
                readBNO055();
                commsTick();
            }
            else if (cmd == "PAD") {
                Serial.println("Transition to PAD");
                gState = STATE_PAD;
                colorRGB(0, 0, 0);
                colorRGB(255, 0, 0);
            }
            else {
                Serial.println("Unknown command");
            }
        }

        break;
    }
    
    case STATE_PAD: { 
        colorRGB(0, 0, 0);
        colorRGB(255, 0, 0);

        madgwickState.beta = 0.05f;
        altitudeFilter.alpha= 0.9f;

        if ( now - lastSlowSample >= SLOW_SAMPLE_INTERVAL_MS) {
            lastSlowSample =  now;
            processSlowSensors();
        }

        if ( now - lastFastSample >= FAST_SAMPLE_INTERVAL_MS) {
            lastFastSample =  now;
            totalAccel = processFastSensors();
            if (sil_command && now - lastSilPrintMs >= 500) {
                lastSilPrintMs = now;
                Serial.printf("[SIL][PAD] t=%lu ms | alt=%.2f m | accel=%.2f m/s2 | vz=%.2f m/s | thresh=%.2f m/s2\n",
                    bnoData.timestamp, altitudeFilter.filteredAltitude,
                    totalAccel, altitudeFilter.verticalVelocity, LAUNCH_ACCEL_THRESHOLD_MS2);
            }
        }

        commsTick();

        if (totalAccel > LAUNCH_ACCEL_THRESHOLD_MS2) {
            if (accelStartMs == 0) {
                accelStartMs =  now;
            }

            if (( now - accelStartMs) >= 500) {
                if (sil_command) Serial.printf("[SIL] PAD -> ASCENT | t=%lu ms | accel=%.2f m/s2\n", now, totalAccel);
                gState = STATE_ASCENT;
                colorRGB(0, 0, 0);
                colorRGB(0, 255, 0);
            }
        }

        else {
            accelStartMs = 0;
        }

        break;
    }

    case STATE_ASCENT: {

        madgwickState.beta = 0.005f;
        altitudeFilter.alpha= 1.0f;        

        if (altitudeFilter.verticalAccel > LAUNCH_ACCEL_THRESHOLD_MS2 || altitudeFilter.verticalVelocity > MACH_VELOCITY_THRESHOLD_MS2) {
            altitudeFilter.alpha = 0.999f; 
        }    

        else {
           altitudeFilter.alpha = 0.95f;
        }

        if (now - lastFastSample >= FAST_SAMPLE_INTERVAL_MS) {
            lastFastSample = now;
            processFastSensors();
            if (sil_command && now - lastSilPrintMs >= 500) {
                lastSilPrintMs = now;
                Serial.printf("[SIL][ASCENT] t=%lu ms | alt=%.2f m | maxAlt=%.2f m | vz=%.2f m/s | az=%.2f m/s2\n",
                    bnoData.timestamp, altitudeFilter.filteredAltitude, maxAltitude,
                    altitudeFilter.verticalVelocity, altitudeFilter.verticalAccel);
            }
        }

        if (now - lastSlowSample >= SLOW_SAMPLE_INTERVAL_MS) {
            lastSlowSample = now;
            processSlowSensors();

            if (altitudeFilter.filteredAltitude > maxAltitude) {
                maxAltitude = altitudeFilter.filteredAltitude;
            }
        }

        commsTick();

        if (altitudeFilter.filteredAltitude < (maxAltitude - 1.5)) {

            if (altitudeStartMs == 0) {
                altitudeStartMs = now;
            }

            if (now - altitudeStartMs >= 500) {
                if (sil_command) Serial.printf("[SIL] ASCENT -> EJECTION | t=%lu ms | alt=%.2f m | maxAlt=%.2f m\n",
                    now, altitudeFilter.filteredAltitude, maxAltitude);
                gState = STATE_EJECTION;

                colorRGB(0, 0, 0);
                colorRGB(255, 255, 0);
            }
        } 
        else {
            altitudeStartMs = 0;
        }
        
        break;
    }

    case STATE_EJECTION: {

        if (now - lastFastSample >= FAST_SAMPLE_INTERVAL_MS) {
            lastFastSample = now;
            processFastSensors();
            if (sil_command && now - lastSilPrintMs >= 500) {
                lastSilPrintMs = now;
                uint32_t ejElapsed = (stableStartMs > 0) ? (now - stableStartMs) : 0UL;
                Serial.printf("[SIL][EJECTION] t=%lu ms | alt=%.2f m | elapsed=%lu ms / 2500 ms\n",
                    bnoData.timestamp, altitudeFilter.filteredAltitude, ejElapsed);
            }
        }

        if (now - lastSlowSample >= SLOW_SAMPLE_INTERVAL_MS) {
            lastSlowSample = now;
            processSlowSensors();
        }

        commsTick();

        if (stableStartMs == 0) {
            stableStartMs = now;
        }

        if (now - stableStartMs > 2500) {
            if (sil_command) Serial.printf("[SIL] EJECTION -> CONTROL | t=%lu ms | alt=%.2f m\n",
                now, altitudeFilter.filteredAltitude);
            gState = STATE_CONTROL;

            colorRGB(0, 0, 0);
            colorRGB(0, 0, 255);
        }

        break;
    }

    case STATE_CONTROL: {

        madgwickState.beta = 0.02f;
        altitudeFilter.alpha= 0.9f; 

        if (now - lastFastSample >= FAST_SAMPLE_INTERVAL_MS) {
            lastFastSample = now;
            processFastSensors();
            if (sil_command && now - lastSilPrintMs >= 500) {
                lastSilPrintMs = now;
                Serial.printf("[SIL][CONTROL] t=%lu ms | alt=%.2f m | vz=%.2f m/s | threshold=50.0 m\n",
                    bnoData.timestamp, bmeData.altitude, altitudeFilter.verticalVelocity);
            }
        }

        if (now - lastSlowSample >= SLOW_SAMPLE_INTERVAL_MS) {
            lastSlowSample = now;
            processSlowSensors();
        }

        commsTick();

        if (bmeData.altitude < 50.0f) {
            if (sil_command) Serial.printf("[SIL] CONTROL -> CUTOFF | t=%lu ms | alt=%.2f m\n",
                now, bmeData.altitude);
            gState = STATE_CUTOFF;

            colorRGB(0, 0, 0);
            colorRGB(0, 255, 255);
        }

        break;
    }

    case STATE_CUTOFF: {
        if (now - lastFastSample >= FAST_SAMPLE_INTERVAL_MS) {
            lastFastSample = now;
            processFastSensors();
            if (sil_command && now - lastSilPrintMs >= 500) {
                lastSilPrintMs = now;
                uint32_t stableMs = (landedStartMs > 0) ? (now - landedStartMs) : 0UL;
                Serial.printf("[SIL][CUTOFF] t=%lu ms | alt=%.2f m | delta=%.2f m | stable=%lu ms / 10000 ms\n",
                    bnoData.timestamp, bmeData.altitude,
                    fabsf(bmeData.altitude - lastLandedAlt), stableMs);
            }
        }

        if (now - lastSlowSample >= SLOW_SAMPLE_INTERVAL_MS) {
            lastSlowSample = now;
            processSlowSensors();

            if (fabsf(bmeData.altitude - lastLandedAlt) < 2.0f) {

                if (landedStartMs == 0) {
                    landedStartMs = now;
                }

                if (now - landedStartMs > 10000) {
                    pageBufFlush();
                    if (sil_command) Serial.printf("[SIL] CUTOFF -> RECOVERY | t=%lu ms | alt=%.2f m\n",
                        now, bmeData.altitude);
                    gState = STATE_RECOVERY;
                    
                    colorRGB(0, 0, 0);
                    colorRGB(255, 0, 255);
                }
            }

            else {
                lastLandedAlt = bmeData.altitude;
                landedStartMs = 0;
            }
        }

        commsTick();

        break;
    }

    case STATE_RECOVERY: {

        if ((now % 1000) < 100) {
            colorRGB(0, 255, 0);
        } else {
            colorRGB(0, 0, 0);
        }

        if (sil_command && now - lastSilPrintMs >= 5000) {
            lastSilPrintMs = now;
            Serial.printf("[SIL][RECOVERY] t=%lu ms | alt=%.2f m\n",
                bnoData.timestamp, bmeData.altitude);
        }

        if (now - lastSlowSample >= 20000) {
            lastSlowSample = now;
            readBME280();
            readUblox();
        }

        break;
    }

    default: {
        gState = STATE_RECOVERY; 
        break;
    }

    } 
}

/**
 * @brief Gets the flight state.
 * * Obtains the current flight state.
 */
FlightState flightComputerGetState() { 
    return gState; 
}