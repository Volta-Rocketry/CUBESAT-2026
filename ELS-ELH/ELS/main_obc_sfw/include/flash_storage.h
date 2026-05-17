#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <SPI.h>

extern SPIClass hspi;
extern uint32_t gFlashWriteAddr;

void flashInit();
void flashEraseChip();
void flashWrite(uint32_t addr, const uint8_t* data, uint16_t len);
void flashRead(uint32_t addr, uint8_t* buf, uint32_t len);
void verifyFlashContent(); 
