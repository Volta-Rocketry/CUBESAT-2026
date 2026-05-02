#pragma once

#include <stdint.h>
#include <stdbool.h>

bool FlashInit();
void FlashEraseChip();
void FlashWrite(uint32_t addr, const uint8_t* data, uint16_t len);
void FlashRead(uint32_t addr, uint8_t* buf, uint32_t len);