#include <Arduino.h>

#include "config.h"
#include "encoders.h"
#include "motors.h"
#include "sensors.h"
#include "sequences.h"
#include "wifi_server.h"

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  delay(200);
  Serial.println();
  Serial.println("========================================");
  Serial.println("[BOOT] Drawbot ESP32 - demarrage");
  Serial.println("[BOOT] Baudrate 115200");

  pinMode(LEDU1_PIN, OUTPUT);
  pinMode(LEDU2_PIN, OUTPUT);
  digitalWrite(LEDU1_PIN, LOW);
  digitalWrite(LEDU2_PIN, LOW);

  setupMotors();
  setupEncoders();
  setupSensors();
  setupSequences();
  setupWifiServer();

  digitalWrite(LEDU1_PIN, HIGH);
  Serial.println("[BOOT] Pret");
  Serial.println("========================================");
}

void loop() {
  handleWifiServer();
  updateSequences();
  updateMotors();
}
