#pragma once

#include <Arduino.h>

struct SensorStatus {
  bool imuDetected;
  bool magnetometerDetected;
};

void setupSensors();
SensorStatus getSensorStatus();
bool readHeadingDegrees(float& headingDegrees);
