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
    FlightState flightState;
    readBME280();
    readBMP180();
    readUblox();
    flightState = flightComputerGetState();
    if (flightState != STATE_PAD && flightState != STATE_RECOVERY) {
        recordFastPacket(); // falta agregar bmp al parquete y creo q qmc tmb
    }
}

float processFastSensors() {
    float currentAccel = 0.0f;
    static float verticalVelocity = 0.0f;
    static uint32_t lastTime = 0;
    FlightState flightState;

    uint32_t now = millis();
    float dt = ( now - lastTime ) / 1000.0f;

    if ( lastTime == 0 || dt < 0.001f || dt > 0.1f ) {
        dt = 0.01f;
    }

    lastTime = now;

    if (initSensor.initBNO) {
        readBNO055();
        readBME280();
        flightState = flightComputerGetState();

        verticalVelocity += bnoData.BNO_global_az * dt;
        
        if (verticalVelocity > 500.0f) verticalVelocity = 500.0f;
        if (verticalVelocity < -200.0f) verticalVelocity = -200.0f;

        altitudeFilter.verticalAccel = bnoData.BNO_global_az;
        altitudeFilter.verticalVelocity = verticalVelocity;

        altitudeFilterUpdate(dt);

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

        if (flightState != STATE_PAD && flightState != STATE_RECOVERY) {
            recordFastPacket();
        }
    }

    else if (initSensor.initMPU && !initSensor.initBNO) {
        readMPU6050(); 
        readBME280();
        flightState = flightComputerGetState();
        madgwickUpdate(&madgwickState, &mpuData, dt);

        float verticalAccel = getVerticalAccel(&madgwickState, &mpuData);

        verticalVelocity += verticalAccel * dt;

        if (verticalVelocity > 500.0f) verticalVelocity = 500.0f;
        if (verticalVelocity < -200.0f) verticalVelocity = -200.0f;

        altitudeFilter.verticalAccel = verticalAccel;
        altitudeFilter.verticalVelocity = verticalVelocity;

        altitudeFilterUpdate(dt);

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
        
        if (flightState != STATE_PAD && flightState != STATE_RECOVERY) {
            recordFastPacket();
        }
    }

    return currentAccel;
}