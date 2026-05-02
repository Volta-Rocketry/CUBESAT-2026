#include "flash_storage.h"

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