#include "linear_kalman_nav.h"

KalmanFilterNav::KalmanFilterNav() {
    altitude = 0.0f;
    velocity = 0.0f;

    // Initialization of high uncertainty for rapid convergence at the beginning
    P[0][0] = 10.0f; P[0][1] = 0.0f;
    P[1][0] = 0.0f;  P[1][1] = 10.0f;

    // validate with real data simulations
    Q_accel = 0.5f;  // Accel noise
    R_baro = 2.0f;   // Baro noise
}

void KalmanFilterNav::setInitialAltitude(float init_alt) {
    altitude = init_alt;
}

void KalmanFilterNav::predict(float accel_z_real, float dt) {

    altitude += velocity * dt + 0.5f * accel_z_real * dt * dt;
    velocity += accel_z_real * dt;

    P[0][0] += dt * (2.0f * P[0][1] + dt * P[1][1]) + Q_accel * dt * dt;
    P[0][1] += dt * P[1][1];
    P[1][0] += dt * P[1][1];
    P[1][1] += Q_accel * dt;
}

void KalmanFilterNav::update(float baro_altitude) {

    float y = baro_altitude - altitude;

    float S = P[0][0] + R_baro;
    float K0 = P[0][0] / S; // Gain for altitude
    float K1 = P[1][0] / S; // Gain for vertical velocity

    // Update state estimation
    altitude += K0 * y;
    velocity += K1 * y;

    // Update error covariance matrix
    float P00_temp = P[0][0];
    float P01_temp = P[0][1];

    P[0][0] = (1.0f - K0) * P00_temp;
    P[0][1] = (1.0f - K0) * P01_temp;
    P[1][0] = -K1 * P00_temp + P[1][0];
    P[1][1] = -K1 * P01_temp + P[1][1];
}

float KalmanFilterNav::getAltitude() const {
    return altitude;
}

float KalmanFilterNav::getVelocity() const {
    return velocity;
}