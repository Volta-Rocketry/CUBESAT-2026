#include "sensor.h"

#include "constants.h"
#include "sensors_manager.h"
#include "error_warning.h"
#include "signals.h"
#include "flash_storage.h"
#include "flight_computer.h"
#include "comm_manager.h"
#include "madgwick_filter.h"
#include <Arduino.h>

void processSlowSensors() {
    readBME280();
    readBMP180();
    readUblox();
    recordSlowPacket();
}

float processFastSensors() {
    float currentAccel = 0.0f;
    float verticalVelocity = 0.0f;
    float linearAccel = 0.0f;
    uint32_t lastTime = 0;
    FlightState flightState;

    if (initSensor.initBNO) {
        uint32_t now = millis();

        readBNO055();
        readBME280();
        flightState = flightComputerGetState();

        float dt = ( now - lastTime ) / 1000.0;
        lastTime = now;

        // verticalVelocity += bnoData.linearAccel * dt;

        commsUpdateCAM(bnoData.timestamp, bnoData.BNO_ax, bnoData.BNO_ay, bnoData.BNO_az,
                       bnoData.BNO_gx, bnoData.BNO_gy, bnoData.BNO_gz);
        commsUpdateCTR(bnoData.timestamp, bmeData.altitude, verticalVelocity, 
                        bnoData.BNO_ax, bnoData.BNO_ay, bnoData.BNO_az,
                        bnoData.BNO_gx, bnoData.BNO_gy, bnoData.BNO_gz, 
                        bnoData.BNO_qw, bnoData.BNO_qx, bnoData.BNO_qy, bnoData.BNO_qz, flightState);
        
        currentAccel = sqrtf(
            bnoData.BNO_ax * bnoData.BNO_ax +
            bnoData.BNO_ay * bnoData.BNO_ay +
            bnoData.BNO_az * bnoData.BNO_az);

        if (initSensor.initMPU) {
            readMPU6050();
            madgwickUpdate(&madgwickState, &mpuData, dt);
        }
        recordFastPacket();
    }

    else if (initSensor.initMPU && !initSensor.initBNO) {
        uint32_t now = millis();

        float dt = ( now - lastTime ) / 1000.0;
        lastTime = now;

        readMPU6050(); 
        readBME280();
        flightState = flightComputerGetState();
        madgwickUpdate(&madgwickState, &mpuData, dt);

        float linearAccel = getLinearAccel(&madgwickState, &mpuData);

        verticalVelocity += linearAccel * dt;

        commsUpdateCAM(mpuData.timestamp, mpuData.MPU_ax, mpuData.MPU_ay, mpuData.MPU_az,
                       mpuData.MPU_gx, mpuData.MPU_gy, mpuData.MPU_gz);
        commsUpdateCTR(mpuData.timestamp, bmeData.altitude, verticalVelocity, 
                        mpuData.MPU_ax, mpuData.MPU_ay, mpuData.MPU_az,
                        mpuData.MPU_gx, mpuData.MPU_gy, mpuData.MPU_gz, 
                        madgwickState.q0, madgwickState.q1, madgwickState.q2, madgwickState.q3, flightState);
        
        currentAccel = sqrtf(
            mpuData.MPU_ax * mpuData.MPU_ax +
            mpuData.MPU_ay * mpuData.MPU_ay +
            mpuData.MPU_az * mpuData.MPU_az); 

        recordFastPacket();
    }

    return currentAccel;
}