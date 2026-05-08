#include "flash_storage.h"
#include "comm_manager.h"
#include "constants.h"
#include "error_warning.h"
#include "signals.h"
#include <Arduino.h>
#include <SPI.h>

SPIClass hspi(HSPI);

static void FlashWaitBusy() {
    digitalWrite(FLASH_CS, LOW);
    hspi.transfer(CMD_READ_STATUS);
    while (hspi.transfer(0x00) & 0x01);
    digitalWrite(FLASH_CS, HIGH);
}

static void FlashWriteEnable() {
    digitalWrite(FLASH_CS, LOW);
    hspi.transfer(CMD_WRITE_ENABLE);
    digitalWrite(FLASH_CS, HIGH);
    delayMicroseconds(5);
}

static void FlashWritePage(uint32_t addr, const uint8_t* data, uint16_t len) {
    FlashWriteEnable();

    digitalWrite(FLASH_CS, LOW);
    hspi.transfer(CMD_PAGE_PROGRAM); 

    hspi.transfer((addr >> 16) & 0xFF); 
    hspi.transfer((addr >> 8) & 0xFF);   
    hspi.transfer( addr & 0xFF);

    for (uint16_t i = 0; i < len; i++) {
        hspi.transfer(data[i]);
    }

    digitalWrite(FLASH_CS, HIGH);
    FlashWaitBusy();
}

void FlashInit() {
    pinMode(FLASH_CS, OUTPUT);
    digitalWrite(FLASH_CS, HIGH);
    delay(10);
    
    digitalWrite(FLASH_CS, LOW);
    hspi.transfer(CMD_JEDEC_ID);
    uint8_t manufacturer = hspi.transfer(0x00);
    uint8_t memType     = hspi.transfer(0x00);
    uint8_t capacity     = hspi.transfer(0x00);

    digitalWrite(FLASH_CS, HIGH);
    if (manufacturer == 0xEF && memType == 0x40 && capacity == 0x16) {
        println("Initialized FLASH");
    }
    else {
        CriticalErrorSensor("FLASH not found");
    }
}

void FlashEraseChip() {
    FlashWriteEnable();
    digitalWrite(FLASH_CS, LOW);
    hspi.transfer(CMD_CHIP_ERASE);
    digitalWrite(FLASH_CS, HIGH);
    FlashWaitBusy();
}

void FlashWrite(uint32_t addr, const uint8_t* data, uint16_t len) {
    uint16_t written = 0;
    uint32_t currentAddr = addr;
    const uint8_t* currentData = data;

    while (written < len) {
        uint16_t pageOffset   = currentAddr % FLASH_PAGE_SIZE;
        uint16_t spaceInPage = FLASH_PAGE_SIZE - pageOffset;

        uint16_t toWrite = (len - written) < spaceInPage ? (len - written) : spaceInPage;
        FlashWritePage(currentAddr, currentData, toWrite);

        written += toWrite;
        currentAddr += toWrite;
        currentData += toWrite;
    }
}

void FlashRead(uint32_t addr, uint8_t* buf, uint32_t len) {
    digitalWrite(FLASH_CS, LOW);
    hspi.transfer(CMD_READ_DATA);

    hspi.transfer((addr >> 16) & 0xFF);
    hspi.transfer((addr >> 8)  & 0xFF);
    hspi.transfer( addr & 0xFF);

    for (uint32_t i = 0; i < len; i++) {
        buf[i] = hspi.transfer(0x00);
    }

    digitalWrite(FLASH_CS, HIGH);
}

/**
 * @brief Verifies and prints the content in flash.
 * * Reads all the Flash memory where the data was stored, 
 * identifies what type of packet each one is, and verifies if the information is complete
 */
void VerifyFlashContent() {
    println("Starting FLASH Verification");
    
    if (gFlashWriteAddr == 0) {
        println("FLASH is empty");
        return;
    }

    Serial.printf("Total written data: %lu bytes\n", gFlashWriteAddr);

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
            
            uint16_t crc_calc = crc16_ccitt((uint8_t*)&p, sizeof(FastFlightPacket) - 2);
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
            
            uint16_t crc_calc = crc16_ccitt((uint8_t*)&p, sizeof(SlowFlightPacket) - 2);
            bool ok = (p.checksum == crc_calc);

            Serial.printf("[SLOW] Addr: 0x%06lX | TS: %lu | Pres: %.2f | GPS_Lat: %.6f | CRC: %s\n", 
                          currentAddr, p.timestamp_ms, p.bme.pressure, p.gps.latitude, ok ? "OK" : "ERROR");

            if (!ok) countCorrupt++;
            countSlow++;
            currentAddr += sizeof(SlowFlightPacket);
        } 
        else {
            Serial.printf("[?] Unknown byte in 0x%06lX: 0x%02X (Saltando...)\n", currentAddr, id);
            currentAddr++; 
            countCorrupt++;
        }

        if ((countFast + countSlow) % 50 == 0) delay(5);
    }

    println("Verification results");
    Serial.printf("  - Fast Packets: %lu\n", countFast);
    Serial.printf("  - Slow Packets: %lu\n", countSlow);
    Serial.printf("  - Corrupted Bytes: %lu\n", countCorrupt);
    println("Verification end");
}