#include "sensors.h"

#include <Wire.h>

#include "config.h"

namespace {
SensorStatus status{false, false};

bool probeI2C(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}
}  // namespace

void setupSensors() {
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  status.imuDetected = probeI2C(ADDR_IMU);
  status.magnetometerDetected = probeI2C(ADDR_MAG);

  Serial.printf("[SENSORS] IMU LSM6DS3 %s, MAG LIS3MDL %s\n",
                status.imuDetected ? "detectee" : "non detectee",
                status.magnetometerDetected ? "detecte" : "non detecte");
}

SensorStatus getSensorStatus() {
  return status;
}

bool readHeadingDegrees(float& headingDegrees) {
  // TODO: lire le LIS3MDL, calibrer hard/soft iron, puis convertir en cap Nord.
  // Le fallback logiciel des sequences ne depend pas de la boussole.
  headingDegrees = 0.0f;
  return false;
}
