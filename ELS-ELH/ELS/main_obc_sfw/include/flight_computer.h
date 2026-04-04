#pragma once

#include "constants.h"

typedef struct __attribute__((packed)) {
    uint8_t packet_id;   
    uint32_t timestamp_ms; 
    StructMPU9250 mpu;         
    StructBNO055 bno;        
    StructTransducer transducer;
    uint16_t checksum;      
} FastFlightPacket;

typedef struct __attribute__((packed)) {
    uint8_t packet_id;    
    uint32_t timestamp_ms;  
    StructBME280 bme;        
    StructUblox gps;           
    uint16_t checksum;      
} SlowFlightPacket;

typedef enum {
    STATE_IDLE,
    STATE_INIT,
    STATE_INTEGRATION,
    STATE_PAD,
    STATE_ASCENT,
    STATE_EYECTION,
    STATE_CONTROL,
    STATE_DRAIN,
    STATE_RECOVERY,
    STATE_DOWNLOAD
} FlightState;

void flight_computer_init();
void flight_computer_update();
FlightState flight_computer_get_state();