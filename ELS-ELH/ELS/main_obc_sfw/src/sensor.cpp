#include "sensor.h"

#include "constants.h"
#include "sensors_manager.h"
#include "error_warning.h"
#include "signals.h"
#include "flash_storage.h"
#include "flight_computer.h"
#include "comm_manager.h"
#include <Arduino.h>

void processSlowSensors() {
    readBME280();
    readBMP180();
    readUblox();
    recordSlowPacket();
}

float processFastSensors() {
    float currentAccel = 0.0f;

    if (initSensor.initBNO) {
        readBNO055(); 
        commsUpdateCAM(bnoData.timestamp, bnoData.BNO_ax, bnoData.BNO_ay, bnoData.BNO_az,
                       bnoData.BNO_gx, bnoData.BNO_gy, bnoData.BNO_gz);
        
        currentAccel = sqrtf(
            bnoData.BNO_ax * bnoData.BNO_ax +
            bnoData.BNO_ay * bnoData.BNO_ay +
            bnoData.BNO_az * bnoData.BNO_az); 
            
        if (initSensor.initMPU) {
            readMPU6050(); 
        }
        recordFastPacket();
    }
    else if (initSensor.initMPU && !initSensor.initBNO) {
        readMPU6050(); 
        commsUpdateCAM(mpuData.timestamp, mpuData.MPU_ax, mpuData.MPU_ay, mpuData.MPU_az,
                       mpuData.MPU_gx, mpuData.MPU_gy, mpuData.MPU_gz);
        
        currentAccel = sqrtf(
            mpuData.MPU_ax * mpuData.MPU_ax +
            mpuData.MPU_ay * mpuData.MPU_ay +
            mpuData.MPU_az * mpuData.MPU_az); 

        recordFastPacket();
    }

    return currentAccel;
}