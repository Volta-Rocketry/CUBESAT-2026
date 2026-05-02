#pragma once

#include "constants.h"

void InitPCB();
void println(const char* message);
void ColorRGB(int r, int g, int b);
void PWMBuzzer(unsigned int f, unsigned long d);