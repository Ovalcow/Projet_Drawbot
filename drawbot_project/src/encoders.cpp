#include "encoders.h"

#include "config.h"

namespace {
volatile long leftTicks = 0;
volatile long rightTicks = 0;

void IRAM_ATTR onLeftEncoderA() {
  const bool a = digitalRead(ENC_G_CH_A_PIN);
  const bool b = digitalRead(ENC_G_CH_B_PIN);
  leftTicks += (a == b) ? 1 : -1;
}

void IRAM_ATTR onRightEncoderA() {
  const bool a = digitalRead(ENC_D_CH_A_PIN);
  const bool b = digitalRead(ENC_D_CH_B_PIN);
  rightTicks += (a == b) ? 1 : -1;
}
}  // namespace

void setupEncoders() {
  pinMode(ENC_G_CH_A_PIN, INPUT_PULLUP);
  pinMode(ENC_G_CH_B_PIN, INPUT_PULLUP);
  pinMode(ENC_D_CH_A_PIN, INPUT_PULLUP);
  pinMode(ENC_D_CH_B_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENC_G_CH_A_PIN), onLeftEncoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_D_CH_A_PIN), onRightEncoderA, CHANGE);
  resetEncoders();
  Serial.println("[ENCODERS] Encodeurs initialises sur CH_A gauche/droit");
}

void resetEncoders() {
  noInterrupts();
  leftTicks = 0;
  rightTicks = 0;
  interrupts();
}

long getLeftEncoderTicks() {
  noInterrupts();
  const long value = leftTicks;
  interrupts();
  return value;
}

long getRightEncoderTicks() {
  noInterrupts();
  const long value = rightTicks;
  interrupts();
  return value;
}

bool encodersAvailable() {
  return true;
}
