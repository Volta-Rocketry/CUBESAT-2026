#include "serial_com.h"

// ──────────── CRC16CCITT ────────────

static uint16_t crc16_ccitt(const uint8_t* data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t b = 0; b < 8; b++) {
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : (crc << 1);
        }
    }
    return crc;
}

// ──────────── DESERIALIZATION LITTLE-ENDIAN ────────────

static uint32_t readU32Le(const uint8_t* buf, uint8_t pos) {
    return  (uint32_t)buf[pos]
          | ((uint32_t)buf[pos + 1] << 8)
          | ((uint32_t)buf[pos + 2] << 16)
          | ((uint32_t)buf[pos + 3] << 24);
}

static float readFloatLe(const uint8_t* buf, uint8_t pos) {
    uint32_t raw = readU32Le(buf, pos);
    float val;
    memcpy(&val, &raw, sizeof(float));
    return val;
}

// DECODES THE CTR PAYLOAD AND ASIGNS IT TO THE CTRDATA
static void decodeCtrPayload(const uint8_t* p, CommsCtrData& ctrData) {
    uint8_t pos = 0;

    ctrData.timestamp         = readU32Le(p, pos);  pos += 4;
    ctrData.altitude          = readFloatLe(p, pos); pos += 4;
    ctrData.vertical_velocity = readFloatLe(p, pos); pos += 4;
    ctrData.ax = readFloatLe(p, pos); pos += 4;
    ctrData.ay = readFloatLe(p, pos); pos += 4;
    ctrData.az = readFloatLe(p, pos); pos += 4;
    ctrData.gx = readFloatLe(p, pos); pos += 4;
    ctrData.gy = readFloatLe(p, pos); pos += 4;
    ctrData.gz = readFloatLe(p, pos); pos += 4;
    ctrData.qw = readFloatLe(p, pos); pos += 4;
    ctrData.qx = readFloatLe(p, pos); pos += 4;
    ctrData.qy = readFloatLe(p, pos); pos += 4;
    ctrData.qz = readFloatLe(p, pos); pos += 4;

    ctrData.flight_state = (FlightState)p[pos];

}

// ──────────── INITTIALIZATION ────────────
static void replyInitEcho(uint8_t payload_id) {
    uint8_t tx_buf[HEADER_LEN + INIT_SP_PAYLOAD_LEN + CRC_LEN];
    tx_buf[0] = COMM_SYNC_1;
    tx_buf[1] = COMM_SYNC_2;
    tx_buf[2] = ID_INIT_CMD;
    tx_buf[3] = INIT_SP_PAYLOAD_LEN;
    tx_buf[4] = payload_id;
    uint16_t crc = crc16_ccitt(tx_buf, 5);
    tx_buf[5] = crc & 0xFF;
    tx_buf[6] = (crc >> 8) & 0xFF;

    Serial1.write(tx_buf, sizeof(tx_buf));
}

// ──────────── RECEPTION/VERIFICATION/ASIGNATION ────────────
bool commsPoll(CommsCtrData& data)
{    
    while (Serial1.available())
    {
        uint8_t c = Serial1.read();        

        if (rx_idx == 0 && c != COMM_SYNC_1)
            continue;

        if (rx_idx == 1 && c != COMM_SYNC_2)
        {
            rx_idx = 0;
            continue;
        }

        rx_buf[rx_idx++] = c;

        if (rx_idx == HEADER_LEN)
        {
            uint8_t id = rx_buf[2];
            uint8_t payload_len = rx_buf[3];

            bool ok =
                (id == ID_CTR_TP   && payload_len == CTR_TP_PAYLOAD_LEN) ||
                (id == ID_INIT_CMD && payload_len == INIT_SP_PAYLOAD_LEN);

            if (!ok)
            {
                rx_idx = 0;
                expected_length = 0;
                continue;
            }

            expected_length = HEADER_LEN + payload_len + CRC_LEN;
        }

        if (expected_length && rx_idx >= expected_length)
        {
            uint8_t cmd_id = rx_buf[2];

            uint16_t crc_rx =
                rx_buf[expected_length - 2] |
                ((uint16_t)rx_buf[expected_length - 1] << 8);

            uint16_t crc_cal =
                crc16_ccitt(rx_buf, expected_length - 2);

            if (crc_rx == crc_cal)
            {
                if (cmd_id == ID_INIT_CMD)
                {
                    replyInitEcho(rx_buf[4]);
                }
                else if (cmd_id == ID_CTR_TP)
                {
                    decodeCtrPayload(
                        &rx_buf[HEADER_LEN],
                        data
                    );

                    rx_idx = 0;
                    expected_length = 0;

                    return true;
                }
            }

            rx_idx = 0;
            expected_length = 0;
        }
    }

    return false;
}

/*

bool commsPoll(CommsCtrData& data)
{
    if (!Serial1.available())
        return false;

    uint8_t c = Serial1.read();

    if (rx_idx == 0 && c != COMM_SYNC_1)
        return false;

    if (rx_idx == 1 && c != COMM_SYNC_2)
    {
        rx_idx = 0;
        return false;
    }

    rx_buf[rx_idx++] = c;

    if (rx_idx == HEADER_LEN)
    {
        uint8_t id = rx_buf[2];
        uint8_t payload_len = rx_buf[3];

        bool ok =
            (id == ID_CTR_TP   && payload_len == CTR_TP_PAYLOAD_LEN) ||
            (id == ID_INIT_CMD && payload_len == INIT_SP_PAYLOAD_LEN);

        if (!ok)
        {
            rx_idx = 0;
            expected_length = 0;
            return false;
        }

        expected_length = HEADER_LEN + payload_len + CRC_LEN;
    }

    if (expected_length &&
        rx_idx >= expected_length)
    {
        uint8_t cmd_id = rx_buf[2];

        uint16_t crc_rx =
            rx_buf[expected_length - 2] |
            ((uint16_t)rx_buf[expected_length - 1] << 8);

        uint16_t crc_cal =
            crc16_ccitt(rx_buf, expected_length - 2);

        bool telemetry_received = false;

        if (crc_rx == crc_cal)
        {
            if (cmd_id == ID_INIT_CMD)
            {
                replyInitEcho(rx_buf[4]);
            }
            else if (cmd_id == ID_CTR_TP)
            {
                decodeCtrPayload(
                    &rx_buf[HEADER_LEN],
                    data
                );

                telemetry_received = true;
            }
        }
        else
        {
            Serial.printf(
                "[RX ERROR] CRC. Rx:%04X Calc:%04X\n",
                crc_rx,
                crc_cal
            );
        }

        rx_idx = 0;
        expected_length = 0;

        return telemetry_received;
    }

    return false;
}

*/