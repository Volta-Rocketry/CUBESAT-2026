#ifndef KALMAN_FILTER_NAV_H
#define KALMAN_FILTER_NAV_H

class KalmanFilterNav {
private:

    float altitude; // x[0]: Altitude estimated [metros]
    float velocity; // x[1]: Vertical velocity estimated [m/s]

    // Erro covariance matrix
    float P[2][2];

    // Noise (Tune)
    float Q_accel; // Accel noise variance
    float R_baro;  // Baro noise variance

public:

    KalmanFilterNav();

    void setInitialAltitude(float init_alt);
    void predict(float accel_z_real, float dt);
    void update(float baro_altitude);

    float getAltitude() const;
    float getVelocity() const;
};

#endif