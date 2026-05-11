#pragma once

#include "constants.h"

void initPCB();
void println(const char* message);
void colorRGB(int r, int g, int b);
void PWMBuzzer(unsigned int f, unsigned long d);