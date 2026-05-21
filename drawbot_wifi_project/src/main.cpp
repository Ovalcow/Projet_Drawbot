#include <Arduino.h>
#include "config.h"
#include "motors.h"
#include "wifi_server.h"

void setup() {
#if DEBUG_ENABLED
  Serial.begin(115200);
  unsigned long t = millis();
  while (!Serial && millis() - t < 2000);
  DBG("\n=== Drawbot WiFi boot ===");
#endif

  setupMotors();
  stopMotors();
  setupWifi();

  DBG("[Main] Setup OK. Appuyer sur BOOT pour activer le WiFi.");
  DBG("[Main] Reseau : " WIFI_AP_SSID "  |  http://" WIFI_AP_IP);
}

void loop() {
  loopWifi();
}