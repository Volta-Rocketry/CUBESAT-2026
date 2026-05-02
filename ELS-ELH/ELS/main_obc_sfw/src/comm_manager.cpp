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

static uint8_t write_u8(uint8_t* buf, uint8_t pos, uint8_t val) {
    buf[pos] = val;
    return pos + 1;
}

static uint8_t write_u16_le(uint8_t* buf, uint8_t pos, uint16_t val) {
    buf[pos]     = (uint8_t)(val & 0xFF);
    buf[pos + 1] = (uint8_t)((val >> 8) & 0xFF);
    return pos + 2;
}

static uint8_t write_u32_le(uint8_t* buf, uint8_t pos, uint32_t val) {
    buf[pos]     = (uint8_t)(val & 0xFF);
    buf[pos + 1] = (uint8_t)((val >> 8)  & 0xFF);
    buf[pos + 2] = (uint8_t)((val >> 16) & 0xFF);
    buf[pos + 3] = (uint8_t)((val >> 24) & 0xFF);
    return pos + 4;
}

static uint8_t write_float_le(uint8_t* buf, uint8_t pos, float val) {
    uint32_t raw;
    memcpy(&raw, &val, sizeof(float));

    return write_u32_le(buf, pos, raw);
}

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

static void build_flight_frame(uint8_t* frame, const CommsFlightData* d) {
    uint8_t pos = 0;

    pos = write_u8(frame, pos, COMM_SYNC_1);
    pos = write_u8(frame, pos, COMM_SYNC_2);
    pos = write_u8(frame, pos, ID_FLIGHT_SP);
    pos = write_u8(frame, pos, FLIGHT_SP_PAYLOAD_LEN);

    uint8_t payload_start = pos;

    pos = write_u32_le(frame, pos, d->timestamp);
    pos = write_float_le(frame, pos, d->altitude);
    pos = write_u8(frame, pos, (uint8_t)d->flight_state);       

    uint16_t crc = crc16_ccitt(frame, payload_start + FLIGHT_SP_PAYLOAD_LEN);
    pos = write_u16_le(frame, pos, crc);
}

static void send_ctr_frame() {
    uint8_t frame[CTR_TP_FRAME_SIZE];    
    build_ctr_frame(frame, &ctrData);
    Serial1.write(frame, CTR_TP_FRAME_SIZE);   
}

static void send_cam_frame() {
    uint8_t frame[CAM_TP_FRAME_SIZE];         
    build_cam_frame(frame, &camData);
    Serial2.write(frame, CAM_TP_FRAME_SIZE);   
}

static void send_flight_frame() {
    uint8_t frame[FLIGHT_SP_FRAME_SIZE];         
    build_flight_frame(frame, &flightData);
    Serial2.write(frame, FLIGHT_SP_FRAME_SIZE);   
}

static void send_test_frame() {
    Serial1.println("Test message"); 
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

void comms_update_flight(uint32_t timestamp, float altitude,
                      FlightState state) {

    flightData.timestamp = timestamp;
    flightData.altitude = altitude;
    flightData.flight_state = state;
}

void CommsInit() { // Añadir verificación de inicialización
    Serial1.begin(BAUD_RATE, SERIAL_8N1, CTR_RX, CTR_TX);
    Serial2.begin(BAUD_RATE, SERIAL_8N1, CAM_RX,  CAM_TX);

    memset(&ctrData, 0, sizeof(CommsCtrData));
    memset(&camData, 0, sizeof(CommsCamData));
    memset(&flightData, 0, sizeof(CommsFlightData));

    LAST_CTR_SEND_MS = 0;
    LAST_CAM_SEND_MS = 0;
    LAST_FLIGHT_SEND_MS = 0;
    LAST_TEST_SEND_MS = 0;

    println("initialized UART1 and UART2 communication");
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

    if (now - LAST_FLIGHT_SEND_MS >= FLIGHT_SP_INTERVAL_MS) {
        LAST_FLIGHT_SEND_MS = now;
        send_flight_frame();
    }
    if (now - LAST_TEST_SEND_MS >= TEST_SP_INTERVAL_MS) {
        LAST_TEST_SEND_MS = now;
        send_test_frame();
    }
}