#pragma once

#include "global.h"

// ──────────── SERIAL COM WITH MAIN OBC ────────────
#define ID_CTR_TP           0x10   // ID CONTROL OBC PACKAGE
#define ID_INIT_CMD         0xF0   // ID INITIALIZATION PACKAGE
#define INIT_SP_PAYLOAD_LEN 1
#define CTR_TP_PAYLOAD_LEN  53
#define COMM_SYNC_1         0xAA
#define COMM_SYNC_2         0x55
#define BAUD_RATE           115200
#define HEADER_LEN          4
#define CRC_LEN             2

// ──────────── PARSER STATE ────────────
static uint8_t  rx_buf[64];
static uint16_t rx_idx = 0;
static uint16_t expected_length = 0;

static uint16_t crc16_ccitt(const uint8_t* data, uint16_t length);
static uint32_t readU32Le(const uint8_t* buf, uint8_t pos);
static float readFloatLe(const uint8_t* buf, uint8_t pos);
static void decodeCtrPayload(const uint8_t* p, CommsCtrData& data);
static void replyInitEcho(uint8_t payload_id);
bool commsPoll(CommsCtrData& data);
