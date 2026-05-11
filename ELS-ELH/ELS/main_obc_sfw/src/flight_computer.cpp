#include "flight_computer.h"
#include "comm_manager.h"
#include "constants.h"
#include "sensors_manager.h"    
#include "error_warning.h"      
#include "signals.h"
#include "flash_storage.h"    
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

/**
 * @brief Calculates the CRC16-CCITT checksum for a data block.
 * * This function implements the CRC-16/CCITT-FALSE algorithm using the polynomial 0x1021.
 * It processes the data byte by byte, applying bitwise XOR and shift operations 
 * to generate a 16-bit cyclic redundancy check.
 * * @param data Pointer to the byte array to be processed.
 * @param length Number of bytes in the data array.
 * @return uint16_t The calculated 16-bit checksum.
 */

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
static void recordFastPacket() {
    FastFlightPacket fast_pkt;
    memset(&fast_pkt, 0, sizeof(FastFlightPacket));
    fast_pkt.packet_id = 0x01;
    fast_pkt.timestamp_ms = millis();
    fast_pkt.mpu = mpuData;
    fast_pkt.bno = bnoData;

    fast_pkt.checksum = crc16CCITT((uint8_t*)&fast_pkt, sizeof(FastFlightPacket) - sizeof(uint16_t));

    pageBufWrite((uint8_t*)&fast_pkt, sizeof(FastFlightPacket));
}

/**
 * @brief Saves the Slow Packet according the data structure.
 * * It obtains the data from the sensors and stores it 
 * according to the structure, for sending to the buffer.
 */
static void recordSlowPacket() {
    SlowFlightPacket slow_pkt;
    memset(&slow_pkt, 0, sizeof(SlowFlightPacket));
    slow_pkt.packet_id = 0x02;
    slow_pkt.timestamp_ms = millis();
    slow_pkt.bme = bmeData;
    slow_pkt.gps = ubloxData;

    slow_pkt.checksum = crc16CCITT((uint8_t*)&slow_pkt, sizeof(SlowFlightPacket) - sizeof(uint16_t));

    pageBufWrite((uint8_t*)&slow_pkt, sizeof(SlowFlightPacket));
}

/**
 * @brief Initializes the flight computer.
 * * Verifies flash space avaiable and initial flight state.
 */
void flightComputerInit() {

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
            dataToInit.id_to_init = ID_CAM_TP;
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
    while (!initSensor.initBNO && !initSensor.initMPU) {
        flashInit();
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

    gFlashWriteAddr = 0;
    gPageBufIdx = 0;

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
    

    gState = STATE_PAD;
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
    static uint32_t landedStartMs  = 0;
    static float    lastLandedAlt  = 0.0f;

    uint32_t now = millis();

    switch (gState) {

    case STATE_IDLE: 

        println("CURRENT STATE: IDLE");

        digitalWrite(LED_BLUE_PIN, HIGH);

        if (now - lastSlowSample >= 1000) {
            lastSlowSample = now;
            readBME280();
            readUblox();
            readBNO055();
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
                    recordSlowPacket();
                }
                else if (cmd == "SAVE FAST DATA") {
                    Serial.println("Saving FAST packet");
                    recordFastPacket();
                }
                else if (cmd == "DOWNLOAD") {
                    Serial.println("Downloading data to FLASH");
                    //// download_flash_to_sd();
                }
                else if (cmd == "ERASE") {
                    Serial.println("Erasing FLASH");
                    flashEraseChip();
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

    case STATE_INTEGRATION: // should be deleted
        if (now - lastSlowSample >= 5000) {
            lastSlowSample = now;
            readBME280();
            recordSlowPacket();
        }
        break;

    case STATE_PAD:

        if (now - lastSlowSample >= 10000) {
            lastSlowSample = now;

            readBME280();
            readUblox();
            recordSlowPacket();
        }
        if (now - lastFastSample >= FAST_SAMPLE_INTERVAL_MS) { 
            lastFastSample = now;
            readBNO055(); 
            //commsUpdateCAM(bnoData.timestamp, bnoData.BNO_ax, bnoData.BNO_ay, bnoData.BNO_az,
            //                bnoData.BNO_gx, bnoData.BNO_gy, bnoData.BNO_gz); // sensor change to MPU6050 according to initialization
            
            commsUpdateCAM(10, 11, 12, 13,
                            14, 15, 16);
            
            float total_accel = sqrtf(
                mpuData.MPU_ax * mpuData.MPU_ax +
                mpuData.MPU_ay * mpuData.MPU_ay +
                mpuData.MPU_az * mpuData.MPU_az
            ); // should be BNO instead of mpu
        
        commsTick();

            if (total_accel > LAUNCH_ACCEL_THRESHOLD_MS2) {
                if (accelStartMs == 0) accelStartMs = now;
                
                if ((now - accelStartMs) >= 500) {
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
            readBNO055();
            recordFastPacket(); // 100 Hz
            commsUpdateCAM(bnoData.timestamp, bnoData.BNO_ax, bnoData.BNO_ay, bnoData.BNO_az,
                            bnoData.BNO_gx, bnoData.BNO_gy, bnoData.BNO_gz);

        }
        if (now - lastSlowSample >= SLOW_SAMPLE_INTERVAL_MS) {
            lastSlowSample = now;
            readBME280();
            readUblox();
            recordSlowPacket(); // 1 Hz

        commsTick();          

            if (bmeData.altitude < maxAltitude) {
                if (altitudeStartMs == 0) altitudeStartMs = now;
                
                if (now - altitudeStartMs >= 500) {
                    println("Apogee detected. Transition to EJECTION.");
                    gState = STATE_EYECTION;
                    digitalWrite(LED_RED_PIN, LOW);
                    digitalWrite(LED_GREEN_PIN, HIGH);
                }
            } 
            else {
                if (bmeData.altitude > maxAltitude) maxAltitude = bmeData.altitude;
                altitudeStartMs = 0;
            }
        }
        break;

    case STATE_EYECTION:
        if (now - lastFastSample >= FAST_SAMPLE_INTERVAL_MS) {
            lastFastSample = now;
            readBNO055();
            recordFastPacket();

            float total_gyro = sqrtf(
                mpuData.MPU_gx * mpuData.MPU_gx +
                mpuData.MPU_gy * mpuData.MPU_gy +
                mpuData.MPU_gz * mpuData.MPU_gz
            );
            
            // IMPLEMENTAR DETECCION DE KILL SWITCHES Y DELAY
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
            readBME280();
            readUblox();
            recordSlowPacket();
        }
        break;

    case STATE_CONTROL:

        if (now - lastFastSample >= FAST_SAMPLE_INTERVAL_MS) {
            lastFastSample = now;
            readBNO055();
            recordFastPacket();
        }
        if (now - lastSlowSample >= SLOW_SAMPLE_INTERVAL_MS) {
            lastSlowSample = now;
            readBME280();
            readUblox();
            recordSlowPacket();

            if (bmeData.altitude < 50.0f) {
                println("Drain threshold. Transition to DRAIN");
                gState = STATE_DRAIN;
            }
        }
        break;

    case STATE_DRAIN:
        if (now - lastFastSample >= FAST_SAMPLE_INTERVAL_MS) {
            lastFastSample = now;
            readBNO055();
            recordFastPacket();
        }
        if (now - lastSlowSample >= SLOW_SAMPLE_INTERVAL_MS) {
            lastSlowSample = now;
            readBME280();
            readUblox();
            recordSlowPacket();

            if (fabsf(bmeData.altitude - lastLandedAlt) < 2.0f) {
                if (landedStartMs == 0) landedStartMs = now;
                if (now - landedStartMs > 4000) { 

                    pageBufFlush(); 
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
            readBME280(); 
        }
        break;

    default:
        break;
    }
}

/**
 * @brief Gets the flight state.
 * * Obtains the current flight state.
 */
FlightState flightComputerGetState() { 
    return gState; 
}