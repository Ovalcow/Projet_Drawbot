#pragma once

#include <Arduino.h>

void setupEncoders();
void resetEncoders();
long getLeftEncoderTicks();
long getRightEncoderTicks();
bool encodersAvailable();
