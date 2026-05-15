#pragma once

#include "constants.h"

void madgwickInit(MadgwickState *filter, float beta);
void madgwickUpdate(MadgwickState *filter, const StructMPU6050 *data, float dt);
void quaternionToEuler(const MadgwickState *filter, EulerAngles *euler);
float getLinearAccel(const MadgwickState *filter, const StructMPU6050 *data);