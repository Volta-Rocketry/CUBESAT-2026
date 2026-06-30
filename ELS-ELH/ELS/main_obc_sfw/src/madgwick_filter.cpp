#include <math.h>
#include <stdint.h> 

#include "constants.h"

MadgwickState madgwickState;
EulerAngles eulerAngles;
AltitudeFilter altitudeFilter;

void madgwickInit(MadgwickState *filter, float beta) {

    filter->q0 = 1.0f;
    filter->q1 = 0.0f;
    filter->q2 = 0.0f;
    filter->q3 = 0.0f;
    filter->beta = beta;
}

void madgwickUpdate(MadgwickState *filter, const StructMPU6050 *data, float dt) {

    float q0 = filter->q0;
    float q1 = filter->q1;
    float q2 = filter->q2;
    float q3 = filter->q3;

    float gx = data->MPU_gx;
    float gy = data->MPU_gy;
    float gz = data->MPU_gz;

    float ax = data->MPU_ax;
    float ay = data->MPU_ay;
    float az = data->MPU_az;

    float normA = sqrtf(ax*ax + ay*ay + az*az);

    if (normA < 1e-6f) {

        filter->q0 = q0 + 0.5f * (-q1*gx - q2*gy - q3*gz) * dt;
        filter->q1 = q1 + 0.5f * ( q0*gx + q2*gz - q3*gy) * dt;
        filter->q2 = q2 + 0.5f * ( q0*gy - q1*gz + q3*gx) * dt;
        filter->q3 = q3 + 0.5f * ( q0*gz + q1*gy - q2*gx) * dt;
    
        float normQ = sqrtf(filter->q0*filter->q0 + filter->q1*filter->q1 + filter->q2*filter->q2 + filter->q3*filter->q3);
        filter->q0 /= normQ;
        filter->q1 /= normQ;
        filter->q2 /= normQ;
        filter->q3 /= normQ;
        
    }

    else {

        float invNormA = 1.0f / normA; 
        ax *= invNormA;   ///< componente x de la gravedad
        ay *= invNormA;   ///< componente y de la gravedad
        az *= invNormA;   ///< componente z de la gravedad

        float f1 = 2.0f*(q1*q3 - q0*q2) - ax;         ///< Error en componente X  
        float f2 = 2.0f*(q0*q1 + q2*q3) - ay;         ///< Error en componente Y 
        float f3 = 2.0f*(0.5f - q1*q1 - q2*q2) - az;  ///< Error en componente Z  

        float grad0 = -2.0f*q2*f1 + 2.0f*q1*f2;               ///< Gradiente en componente q0  
        float grad1 =  2.0f*q3*f1 + 2.0f*q0*f2 - 4.0f*q1*f3;  ///< Gradiente en componente q1  
        float grad2 = -2.0f*q0*f1 + 2.0f*q3*f2 - 4.0f*q2*f3;  ///< Gradiente en componente q2  
        float grad3 =  2.0f*q1*f1 + 2.0f*q2*f2;               ///< Gradiente en componente q3  
                                                    
        float norm_grad = sqrtf(grad0*grad0 + grad1*grad1 +
                                grad2*grad2 + grad3*grad3);

        if (norm_grad < 1e-6f) {

            filter->q0 = q0 + 0.5f * (-q1*gx - q2*gy - q3*gz) * dt;
            filter->q1 = q1 + 0.5f * ( q0*gx + q2*gz - q3*gy) * dt;
            filter->q2 = q2 + 0.5f * ( q0*gy - q1*gz + q3*gx) * dt;
            filter->q3 = q3 + 0.5f * ( q0*gz + q1*gy - q2*gx) * dt;
        
            float normQ = sqrtf(filter->q0*filter->q0 + filter->q1*filter->q1 + filter->q2*filter->q2 + filter->q3*filter->q3);
            filter->q0 /= normQ;
            filter->q1 /= normQ;
            filter->q2 /= normQ;
            filter->q3 /= normQ;

        }

        else {

            float invNormGrad = 1.0f / norm_grad;
            grad0 *= invNormGrad;   ///< Gradiente N componente q0
            grad1 *= invNormGrad;   ///< Gradiente N componente q1
            grad2 *= invNormGrad;   ///< Gradiente N componente q2
            grad3 *= invNormGrad;   ///< Gradiente N componente q3

            float qdot0 = 0.5f * (-q1*gx - q2*gy - q3*gz);
            float qdot1 = 0.5f * ( q0*gx + q2*gz - q3*gy);
            float qdot2 = 0.5f * ( q0*gy - q1*gz + q3*gx);
            float qdot3 = 0.5f * ( q0*gz + q1*gy - q2*gx);

            filter->q0 = q0 + (qdot0 - filter->beta * grad0) * dt;
            filter->q1 = q1 + (qdot1 - filter->beta * grad1) * dt;
            filter->q2 = q2 + (qdot2 - filter->beta * grad2) * dt;
            filter->q3 = q3 + (qdot3 - filter->beta * grad3) * dt;

            float normQ = sqrtf(filter->q0 * filter->q0 +
                         filter->q1 * filter->q1 +
                         filter->q2 * filter->q2 +
                         filter->q3 * filter->q3);

            if (normQ < 1e-6f) {
                filter->q0 = 1.0f;
                filter->q1 = 0.0f;
                filter->q2 = 0.0f;
                filter->q3 = 0.0f;
                return;
            }

            float invNormQ = 1.0f / normQ;
            filter->q0 *= invNormQ;
            filter->q1 *= invNormQ;
            filter->q2 *= invNormQ;
            filter->q3 *= invNormQ;
        }
    }
}

void quaternionToEuler(const MadgwickState *filter, EulerAngles *euler) {
    float q0 = filter->q0;
    float q1 = filter->q1;
    float q2 = filter->q2;
    float q3 = filter->q3;

    euler->roll = atan2f(2.0f*(q2*q3 + q0*q1), 1.0f - 2.0f*(q1*q1 + q2*q2)) * 57.2957795f;

    float sin_pitch = -2.0f*(q1*q3 - q0*q2);
    if (sin_pitch >  1.0f) {
        sin_pitch =  1.0f;
    }
    else if (sin_pitch < -1.0f) {
        sin_pitch = -1.0f;
    }

    euler->pitch = asinf(sin_pitch) * 57.2957795f;
    euler->yaw = atan2f(2.0f*(q1*q2 + q0*q3), 1.0f - 2.0f*(q2*q2 + q3*q3)) * 57.2957795f;

}

float getVerticalAccel(const MadgwickState *filter, const StructMPU6050 *data) {

    float q0 = filter->q0;
    float q1 = filter->q1;
    float q2 = filter->q2;
    float q3 = filter->q3;

    float gx = 2.0f * (q1*q3 - q0*q2);
    float gy = 2.0f * (q0*q1 + q2*q3);
    float gz = 1.0f - 2.0f * (q1*q1 + q2*q2);

    float g_mss = 9.80665f;
    float gx_mss = gx * g_mss;
    float gy_mss = gy * g_mss;
    float gz_mss = gz * g_mss;

    float linearAccelX = data->MPU_ax - gx_mss;
    float linearAccelY = data->MPU_ay - gy_mss;
    float linearAccelZ = data->MPU_az - gz_mss;

    float verticalAccelMs2 = (gx * linearAccelX) + (gy * linearAccelY) + (gz * linearAccelZ);

    return verticalAccelMs2;
}

void altitudeFilterUpdate(float dt) {

    float predictedAltitude = altitudeFilter.filteredAltitude + (altitudeFilter.verticalVelocity * dt) + (0.5f * altitudeFilter.verticalAccel * dt * dt);
    float predictedVelocity = altitudeFilter.verticalVelocity + (altitudeFilter.verticalAccel * dt);

    altitudeFilter.filteredAltitude = (altitudeFilter.alpha * predictedAltitude) + ((1.0f - altitudeFilter.alpha) * bmeData.altitude);
    
    altitudeFilter.verticalVelocity = altitudeFilter.alpha * predictedVelocity + (1.0f - altitudeFilter.alpha) * ((altitudeFilter.filteredAltitude - predictedAltitude) / dt);
}