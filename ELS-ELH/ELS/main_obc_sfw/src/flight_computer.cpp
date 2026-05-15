#include "flight_computer.h"
#include "comm_manager.h"
#include "constants.h"
#include "sensors_manager.h"    
#include "error_warning.h"      
#include "signals.h"
#include "flash_storage.h"    
#include "sensor.h"
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
void recordFastPacket() {
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
void recordSlowPacket() {
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
    static uint32_t drainStartMs   = 0;
    static uint32_t landedStartMs  = 0;
    static float    lastLandedAlt  = 0.0f;
    float totalAccel = 0.0f;
    float totalGyro = 0.0f;

    unsigned long now = millis();

    switch (gState) {

    case STATE_IDLE: {

        println("CURRENT STATE: IDLE");

        colorRGB(0, 0, 255);

        if ( now - lastSlowSample >= 1000) {
            lastSlowSample = now;
            readBME280();
            readUblox();
            readBNO055();
        }

        println("Write a command: ");

        if (Serial.available() > 0) {
            String cmd = Serial.readStringUntil('\n');
            cmd.trim();

            if (cmd == "SAVE SLOW DATA") {
                Serial.println("Saving SLOW packet");
                recordSlowPacket();
            }
            else if (cmd == "SAVE FAST DATA") {
                Serial.println("Saving FAST packet");
                recordFastPacket();
            }
            else if (cmd == "ERASE") {
                Serial.println("Erasing FLASH");
                flashEraseChip();
                gFlashWriteAddr = 0;
                gPageBufIdx = 0;
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

        if ( now - lastSlowSample >= SLOW_SAMPLE_INTERVAL_MS) {
            lastSlowSample =  now;
            processSlowSensors();
        }

        if ( now - lastFastSample >= FAST_SAMPLE_INTERVAL_MS) { 
            lastFastSample =  now;
            totalAccel = processFastSensors(); 
        }

        if (totalAccel > LAUNCH_ACCEL_THRESHOLD_MS2) {
            if (accelStartMs == 0) {
                accelStartMs =  now;
            }
            
            if (( now - accelStartMs) >= 500) { 
                gState = STATE_ASCENT;
                colorRGB(0, 0, 0);
                colorRGB(0, 255, 0);
            }
        } 
        else {
            accelStartMs = 0;
        }

        commsTick();
        break;
    }

    case STATE_ASCENT: {

        if (now - lastFastSample >= FAST_SAMPLE_INTERVAL_MS) {
            lastFastSample = now;
            processFastSensors();
        }

        if (now - lastSlowSample >= SLOW_SAMPLE_INTERVAL_MS) {
            lastSlowSample = now;
            processSlowSensors();
            
            if (bmeData.altitude > maxAltitude) {
                maxAltitude = bmeData.altitude;
            }
        }

        commsTick();          

        if (bmeData.altitude < (maxAltitude - 1.5)) { 
            
            if (altitudeStartMs == 0) {
                altitudeStartMs = now;
            }
            
            if (now - altitudeStartMs >= 500) {
                gState = STATE_EYECTION; 

                colorRGB(0, 0, 0);
                colorRGB(255, 255, 0);
            }
        } 
        else {
            altitudeStartMs = 0;
        }
        
        break;
    }

    case STATE_EYECTION: {
        if (now - lastFastSample >= FAST_SAMPLE_INTERVAL_MS) {
            lastFastSample = now;
            processFastSensors(); 

            totalGyro = sqrtf(
                bnoData.BNO_gx * bnoData.BNO_gx +
                bnoData.BNO_gy * bnoData.BNO_gy +
                bnoData.BNO_gz * bnoData.BNO_gz
            );
            
            if (totalGyro < 20.0f) {  
                if (stableStartMs == 0) {
                    stableStartMs = now;
                }
                
                if (now - stableStartMs > 2000) {
                    gState = STATE_CONTROL;
                    
                    colorRGB(0, 0, 0);
                    colorRGB(0, 0, 255);
                }
            } 
            else {
                stableStartMs = 0;
            }
        }
        
        if (now - lastSlowSample >= SLOW_SAMPLE_INTERVAL_MS) {
            lastSlowSample = now;
            processSlowSensors();
        }

        commsTick();

        break;
    }

    case STATE_CONTROL: {
        if (now - lastFastSample >= FAST_SAMPLE_INTERVAL_MS) {
            lastFastSample = now;
            processFastSensors();
        }
        
        if (now - lastSlowSample >= SLOW_SAMPLE_INTERVAL_MS) {
            lastSlowSample = now;
            processSlowSensors();
        }

        commsTick();

        if (bmeData.altitude < 50.0f) {
            if (drainStartMs == 0) {
                drainStartMs = now;
            }
            
            if (now - drainStartMs > 1000) {
                gState = STATE_DRAIN;
                
                colorRGB(0, 0, 0);
                colorRGB(0, 255, 255);
            }
        } 
        else {
            drainStartMs = 0;
        }

        break;
    }

    case STATE_DRAIN: {
        if (now - lastFastSample >= FAST_SAMPLE_INTERVAL_MS) {
            lastFastSample = now;
            processFastSensors();
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

        if (now - lastSlowSample >= 20000) {
            lastSlowSample = now;
            
            readBME280();
            readUblox();

        }
        commsTick(); 

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