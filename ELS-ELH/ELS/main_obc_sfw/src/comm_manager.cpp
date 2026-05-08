#include <Arduino.h>
#include <string.h>
#include "constants.h"
#include "comm_manager.h"
#include "signals.h"

CommsCtrData ctrData;
CommsCamData camData;
CommsFlightData flightData;

static uint32_t LAST_CTR_SEND_MS = 0;
static uint32_t LAST_CAM_SEND_MS = 0;
static uint32_t LAST_FLIGHT_SEND_MS = 0;
static uint32_t LAST_TEST_SEND_MS = 0;

/**
 * @brief Calculates the CRC16-CCITT checksum for a data block.
 * * This function implements the CRC-16/CCITT-FALSE algorithm using the polynomial 0x1021.
 * It processes the data byte by byte, applying bitwise XOR and shift operations 
 * to generate a 16-bit cyclic redundancy check.
 * * @param data Pointer to the byte array to be processed.
 * @param length Number of bytes in the data array.
 * @return uint16_t The calculated 16-bit checksum.
 */
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

/**
 * @brief Writes a byte into a buffer and advance the index position.
 * * Serializes an 8-bit data point at the specified position in an array,
 * facilitating the sequential packaging of telemetry frames.
 * * @param buf Pointer to the destination buffer where the data will be stored.
 * @param pos Current index within the buffer.
 * @param val 8-bit value (uint8_t) to be written.
 * @return uint8_t The next available position in the buffer (pos + 1).
 */
static uint8_t write_u8(uint8_t* buf, uint8_t pos, uint8_t val) {
    buf[pos] = val;
    return pos + 1;
}

/**
 * @brief Writes a 16-bit unsigned integer to the buffer using Little Endian format.
 * * This function serializes a uint16_t value into two consecutive bytes in the buffer.
 * It places the Low Byte at the current position and the High Byte at 
 * the following position, then advances the index by two.
 * * @param buf Pointer to the destination data buffer.
 * @param pos Current write index within the buffer.
 * @param val The 16-bit value to be serialized.
 * @return uint8_t The updated position index (pos + 2).
 */
static uint8_t write_u16_le(uint8_t* buf, uint8_t pos, uint16_t val) {
    buf[pos]     = (uint8_t)(val & 0xFF);
    buf[pos + 1] = (uint8_t)((val >> 8) & 0xFF);
    return pos + 2;
}

/**
 * @brief Writes a 32-bit unsigned integer to the buffer using Little Endian format.
 * * This function serializes a uint32_t value into four consecutive bytes. 
 * It breaks down the 32-bit word into bytes, storing the Least Significant Byte 
 * at the first position and the Most Significant Byte at the last, 
 * then increments the buffer index by four.
 * * @param buf Pointer to the destination data buffer.
 * @param pos Current write index within the buffer.
 * @param val The 32-bit value to be serialized (e.g., system timestamp).
 * @return uint8_t The updated position index (pos + 4).
 */
static uint8_t write_u32_le(uint8_t* buf, uint8_t pos, uint32_t val) {
    buf[pos]     = (uint8_t)(val & 0xFF);
    buf[pos + 1] = (uint8_t)((val >> 8)  & 0xFF);
    buf[pos + 2] = (uint8_t)((val >> 16) & 0xFF);
    buf[pos + 3] = (uint8_t)((val >> 24) & 0xFF);
    return pos + 4;
}

/**
 * @brief Writes a 32-bit floating-point number to the buffer in Little Endian format.
 * * This function converts a float to its raw binary representation (IEEE 754) 
 * and serializes it into four consecutive bytes. It reuses the write_u32_le 
 * logic to ensure the bytes are stored from Least Significant to Most Significant.
 * * @param buf Pointer to the destination data buffer.
 * @param pos Current write index within the buffer.
 * @param val The float value to be serialized (e.g., altitude, acceleration).
 * @return uint8_t The updated position index (pos + 4).
 */
static uint8_t write_float_le(uint8_t* buf, uint8_t pos, float val) {
    uint32_t raw;
    memcpy(&raw, &val, sizeof(float));

    return write_u32_le(buf, pos, raw);
}

/**
 * @brief Constructs a serialized telemetry frame for the Control OBC (CTR).
 * * This function assembles a complete binary packet by sequential writing:
 * 1. Header: Synchronization bytes and Packet ID.
 * 2. Metadata: Payload length.
 * 3. Payload: Telemetry data (timestamp, altitude, IMU data, flight state) 
 * converted to Little Endian format.
 * 4. Integrity: Appends a CRC16-CCITT checksum at the end of the frame.
 * * @param frame Pointer to the buffer where the binary frame will be stored.
 * @param d Pointer to the source CommsCtrData structure containing telemetry.
 */
static void build_ctr_frame(uint8_t* frame, const CommsCtrData* d) {
    uint8_t pos = 0;

    pos = write_u8(frame, pos, COMM_SYNC_1);
    pos = write_u8(frame, pos, COMM_SYNC_2);
    pos = write_u8(frame, pos, ID_CTR_TP);
    pos = write_u8(frame, pos, CTR_TP_PAYLOAD_LEN);

    uint8_t payload_start = pos;

    pos = write_u32_le(frame, pos, d->timestamp);        
    pos = write_float_le(frame, pos, d->altitude);   
    pos = write_float_le(frame, pos, d->vertical_velocity);
    pos = write_float_le(frame, pos, d->ax);          
    pos = write_float_le(frame, pos, d->ay);               
    pos = write_float_le(frame, pos, d->az);            
    pos = write_float_le(frame, pos, d->gx);             
    pos = write_float_le(frame, pos, d->gy);             
    pos = write_float_le(frame, pos, d->gz);   
    pos = write_float_le(frame, pos, d->qw);    
    pos = write_float_le(frame, pos, d->qx);        
    pos = write_float_le(frame, pos, d->qy);         
    pos = write_float_le(frame, pos, d->qz);
    pos = write_u8(frame, pos, (uint8_t)d->flight_state);

    uint16_t crc = crc16_ccitt(frame, payload_start + CTR_TP_PAYLOAD_LEN);
    pos = write_u16_le(frame, pos, crc);
}

/**
 * @brief Constructs a serialized telemetry frame for the Camera (CAM).
 * * This function assembles a binary packet specifically designed for the camera module.
 * It follows a standard telemetry structure:
 * 1. Header: Sync bytes (0xAA, 0xAF) and Camera Packet ID.
 * 2. Metadata: Payload length for the camera data.
 * 3. Payload: High-frequency motion data (Timestamp, Accel, Gyro) in Little Endian.
 * 4. Integrity: Appends a CRC16-CCITT checksum for error detection.
 * * @param frame Pointer to the buffer where the binary frame will be stored.
 * @param d Pointer to the source CommsCamData structure.
 */
static void build_cam_frame(uint8_t* frame, const CommsCamData* d) {
    uint8_t pos = 0;

    pos = write_u8(frame, pos, COMM_SYNC_1);
    pos = write_u8(frame, pos, COMM_SYNC_2);
    pos = write_u8(frame, pos, ID_CAM_TP);
    pos = write_u8(frame, pos, CAM_TP_PAYLOAD_LEN);

    uint8_t payload_start = pos;

    pos = write_u32_le(frame, pos, d->timestamp);
    pos = write_float_le(frame, pos, d->ax);          
    pos = write_float_le(frame, pos, d->ay);               
    pos = write_float_le(frame, pos, d->az);            
    pos = write_float_le(frame, pos, d->gx);             
    pos = write_float_le(frame, pos, d->gy);             
    pos = write_float_le(frame, pos, d->gz);

    uint16_t crc = crc16_ccitt(frame, payload_start + CAM_TP_PAYLOAD_LEN);
    pos = write_u16_le(frame, pos, crc);
}

static void build_init_frame(uint8_t* frame, const CommsInitData* d) {
    uint8_t pos = 0;
    pos = write_u8(frame, pos, COMM_SYNC_1);
    pos = write_u8(frame, pos, COMM_SYNC_2);
    pos = write_u8(frame, pos, ID_INIT_CMD);
    pos = write_u8(frame, pos, INIT_SP_PAYLOAD_LEN);

    uint8_t payload_start = pos;
    
    pos = write_u8(frame, pos, d->id_to_init); 

    uint16_t crc = crc16_ccitt(frame, payload_start + INIT_SP_PAYLOAD_LEN);
    pos = write_u16_le(frame, pos, crc);
}

/**
 * @brief Orchestrates the assembly and transmission of the Control (CTR) telemetry frame.
 * * This function performs the following steps:
 * 1. Allocates a local byte buffer of size CTR_TP_FRAME_SIZE.
 * 2. Calls build_ctr_frame() to serialize the current global 'ctrData' into the buffer.
 * 3. Transmits the raw binary packet through the Serial1 interface.
 * * @note This function depends on the global instance 'ctrData' being updated 
 * prior to the call.
 */
static void send_ctr_frame() {
    uint8_t frame[CTR_TP_FRAME_SIZE];    
    build_ctr_frame(frame, &ctrData);
    Serial1.write(frame, CTR_TP_FRAME_SIZE);   
}

/**
 * @brief Orchestrates the assembly and transmission of the Camera (CAM) telemetry frame.
 * * This function performs the following steps:
 * 1. Allocates a local byte buffer of size CAM_TP_FRAME_SIZE.
 * 2. Calls build_cam_frame() to serialize the current global 'camData' into the buffer.
 * 3. Transmits the raw binary packet through the Serial2 interface.
 * * @note This function depends on the global instance 'camData' being updated 
 * prior to the call.
 */
static void send_cam_frame() {
    uint8_t frame[CAM_TP_FRAME_SIZE];         
    build_cam_frame(frame, &camData);
    Serial2.write(frame, CAM_TP_FRAME_SIZE);   
}

static void send_test_frame(HardwareSerial& serialPort, const CommsInitData* d) {
    uint8_t frame[INIT_SP_FRAME_SIZE];    
    build_init_frame(frame, d);

    //------------
    Serial.print("[TX DEBUG] plot: ");
    for(int i = 0; i < INIT_SP_FRAME_SIZE; i++) {
        Serial.printf("%02X ", frame[i]);
    }
    Serial.println();
    //-----------

    serialPort.write(frame, INIT_SP_FRAME_SIZE);   
}

void comms_update_ctr(uint32_t timestamp, float altitude, float vertical_velocity,
                      float ax, float ay, float az,
                      float gx, float gy, float gz,
                      float qw, float qx, float qy, float qz,
                      FlightState state) {

    ctrData.timestamp = timestamp;
    ctrData.altitude = altitude;
    ctrData.vertical_velocity = vertical_velocity;
    ctrData.ax = ax; ctrData.ay = ay; ctrData.az = az;
    ctrData.gx = gx; ctrData.gy = gy; ctrData.gz = gz;
    ctrData.qw = qw; ctrData.qx = qx; ctrData.qy = qy;  ctrData.qz = qz;
    ctrData.flight_state = state;
}

void comms_update_cam(uint32_t timestamp,
                      float ax, float ay, float az,
                      float gx, float gy, float gz) {

    camData.timestamp = timestamp;
    camData.ax = ax; camData.ay = ay; camData.az = az;
    camData.gx = gx; camData.gy = gy; camData.gz = gz;
}

/**
 * @brief Initializes and verifies bidirectional communication with a specific module.
 * * This function sends an initialization frame to the target module and waits 
 * for an identical echoed response. It validates the synchronization bytes, 
 * the target ID, and the CRC16 checksum.
 * * @param serialPort HardwareSerial instance (Serial1, Serial2, etc.).
 * @param rxPin GPIO pin for RX.
 * @param txPin GPIO pin for TX.
 * @param d Pointer to the initialization data structure.
 * @return true If the module responds with a valid, non-corrupt frame.
 * @return false If timeout occurs or verification fails.
 */
bool CommsInit(HardwareSerial& serialPort, int rxPin, int txPin, const CommsInitData* d) {
    serialPort.begin(BAUD_RATE, SERIAL_8N1, rxPin, txPin);
    
    while(serialPort.available()) { serialPort.read(); }

    Serial.printf("Initializing communications for ID: 0x%02X\n", d->id_to_init);
    send_init_frame(serialPort, d);

    uint32_t start_time = millis();
    uint8_t rx_buf[INIT_SP_FRAME_SIZE];
    uint8_t rx_index = 0;

    while (millis() - start_time < 5000) {
        while (serialPort.available() > 0) {
            uint8_t c = serialPort.read();
            if (rx_index == 0 && c != COMM_SYNC_1) continue;
            if (rx_index == 1 && c != COMM_SYNC_2) { rx_index = 0; continue; }

            rx_buf[rx_index++] = c;

            if (rx_index >= INIT_SP_FRAME_SIZE) {

                if (rx_buf[2] == ID_INIT_CMD && rx_buf[4] == d->id_to_init) {
                    
                    Serial.print("[RX DEBUG] Buffer plot: ");
                    for(int i = 0; i < INIT_SP_FRAME_SIZE; i++) {
                        Serial.printf("%02X ", rx_buf[i]); 
                    }
                    Serial.println();
                    uint16_t crc_rx = rx_buf[5] | (rx_buf[6] << 8);
                    uint16_t crc_calc = crc16_ccitt(rx_buf, 5);

                    if (crc_rx == crc_calc) {
                        Serial.println("[COMMS OK] Package sended and recived.");
                        return true; 
                    } else {
                        Serial.println("[COMMS ERROR] MAIN OBC didnt recived the sended package.");
                        return false;
                    }
                }
                rx_index = 0;
            }
        }
    }
        
    Serial.println("[COMMS ERROR] Timeout. MAIN OBC didnt recived the sended package on time.");
    return false;
}

void CommsTick() {
    uint32_t now = millis();

    if (now - LAST_CTR_SEND_MS >= CTR_TP_INTERVAL_MS) {
        LAST_CTR_SEND_MS = now;
        send_ctr_frame();
    }

    if (now - LAST_CAM_SEND_MS >= CAM_TP_INTERVAL_MS) {
        LAST_CAM_SEND_MS = now;
        send_cam_frame();
    }

}