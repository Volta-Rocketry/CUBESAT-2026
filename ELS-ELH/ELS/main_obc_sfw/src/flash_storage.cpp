#include "flash_storage.h"
#include "constants.h"
#include <Arduino.h>
#include <SPI.h>

#define CMD_WRITE_ENABLE 0x06
#define CMD_CHIP_ERASE 0xC7
#define CMD_PAGE_PROGRAM 0x02
#define CMD_READ_DATA 0x03
#define CMD_READ_STATUS 0x05
#define CMD_JEDEC_ID 0x9F

#define PIN_CS_FLASH FLASH_CS

extern SPIClass hspi;

static void flash_wait_busy() {
    digitalWrite(PIN_CS_FLASH, LOW);
    hspi.transfer(CMD_READ_STATUS);
    while (hspi.transfer(0x00) & 0x01);
    digitalWrite(PIN_CS_FLASH, HIGH);
}

static void flash_write_enable() {
    digitalWrite(PIN_CS_FLASH, LOW);
    hspi.transfer(CMD_WRITE_ENABLE);
    digitalWrite(PIN_CS_FLASH, HIGH);
    delayMicroseconds(5);
}

static void flash_write_page(uint32_t addr, const uint8_t* data, uint16_t len) {
    flash_write_enable();

    digitalWrite(PIN_CS_FLASH, LOW);
    hspi.transfer(CMD_PAGE_PROGRAM); 

    hspi.transfer((addr >> 16) & 0xFF); 
    hspi.transfer((addr >> 8) & 0xFF);   
    hspi.transfer( addr & 0xFF);

    for (uint16_t i = 0; i < len; i++) {
        hspi.transfer(data[i]);
    }

    digitalWrite(PIN_CS_FLASH, HIGH);
    flash_wait_busy();
}

bool flash_init() {
    pinMode(PIN_CS_FLASH, OUTPUT);
    digitalWrite(PIN_CS_FLASH, HIGH);

    digitalWrite(PIN_CS_FLASH, LOW);
    hspi.transfer(CMD_JEDEC_ID);
    uint8_t manufacturer = hspi.transfer(0x00);
    uint8_t mem_type     = hspi.transfer(0x00);
    uint8_t capacity     = hspi.transfer(0x00);

    digitalWrite(PIN_CS_FLASH, HIGH);
    return (manufacturer == 0xEF && mem_type == 0x40 && capacity == 0x18);
}

void flash_erase_chip() {
    flash_write_enable();
    digitalWrite(PIN_CS_FLASH, LOW);
    hspi.transfer(CMD_CHIP_ERASE);
    digitalWrite(PIN_CS_FLASH, HIGH);
    flash_wait_busy();
}

void flash_write(uint32_t addr, const uint8_t* data, uint16_t len) {
    uint16_t written = 0;
    uint32_t current_addr = addr;
    const uint8_t* current_data = data;

    while (written < len) {
        uint16_t page_offset   = current_addr % FLASH_PAGE_SIZE;
        uint16_t space_in_page = FLASH_PAGE_SIZE - page_offset;

        uint16_t to_write = (len - written) < space_in_page ? (len - written) : space_in_page;
        flash_write_page(current_addr, current_data, to_write);

        written += to_write;
        current_addr += to_write;
        current_data += to_write;
    }
}

void flash_read(uint32_t addr, uint8_t* buf, uint32_t len) {
    digitalWrite(PIN_CS_FLASH, LOW);
    hspi.transfer(CMD_READ_DATA);

    hspi.transfer((addr >> 16) & 0xFF);
    hspi.transfer((addr >> 8)  & 0xFF);
    hspi.transfer( addr & 0xFF);

    for (uint32_t i = 0; i < len; i++) {
        buf[i] = hspi.transfer(0x00);
    }

    digitalWrite(PIN_CS_FLASH, HIGH);
}