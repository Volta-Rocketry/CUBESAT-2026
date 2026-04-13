#pragma once

#include <stdint.h>
#include <stdbool.h>

#define FLASH_PAGE_SIZE 256UL
#define FLASH_TOTAL_BYTES 16777216UL

bool flash_init();
void flash_erase_chip();
void flash_write(uint32_t addr, const uint8_t* data, uint16_t len);
void flash_read(uint32_t addr, uint8_t* buf, uint32_t len);