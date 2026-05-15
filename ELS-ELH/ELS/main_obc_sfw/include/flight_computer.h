#pragma once

#include "constants.h"
#include "flash_storage.h"
#include "sensors_manager.h"

void recordFastPacket();
void recordSlowPacket();
void flightComputerInit();
void flightComputerUpdate();
FlightState flightComputerGetState();
