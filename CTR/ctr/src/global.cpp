#include "global.h"

BluetoothSerial SerialBT;

RWController rwController =
{
    .kp = 5.0f,
    .kd = 70.0f,
    .prev_error = 0.0f,
    .prev_time = 0
};