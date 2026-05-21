#include "encoders.h"
#include "config.h"

#include <Arduino.h>

// ============================================================
//  Compteurs encodeurs (ticks)
// ============================================================
static volatile long encG = 0;
static volatile long encD = 0;

// ============================================================
//  ISR — mode quadrature (2 voies)
//  Convention de direction utilisée:
//  - Pour chaque front CHANGE sur la voie A, on compare l'état de B.
// ============================================================
void IRAM_ATTR isrEncG_A() {
  // Si CHA == CHB alors +1 sinon -1
  encG += (digitalRead(ENC_G_CH_A) == digitalRead(ENC_G_CH_B)) ? 1 : -1;
}

void IRAM_ATTR isrEncD_A() {
  // Si CDA == CDB alors +1 sinon -1
  encD += (digitalRead(ENC_D_CH_A) == digitalRead(ENC_D_CH_B)) ? 1 : -1;
}

// ============================================================
//  API
// ============================================================
void setupEncoders() {
  pinMode(ENC_G_CH_A, INPUT_PULLUP);
  pinMode(ENC_G_CH_B, INPUT_PULLUP);
  pinMode(ENC_D_CH_A, INPUT_PULLUP);
  pinMode(ENC_D_CH_B, INPUT_PULLUP);

  encG = 0;
  encD = 0;

  // Attacher sur la voie A (moins d'ISR, direction via la voie B)
  attachInterrupt(digitalPinToInterrupt(ENC_G_CH_A), isrEncG_A, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_D_CH_A), isrEncD_A, CHANGE);
}

long getEncG() {
  noInterrupts();
  long v = encG;
  interrupts();
  return v;
}

long getEncD() {
  noInterrupts();
  long v = encD;
  interrupts();
  return v;
}

void resetEncoders() {
  noInterrupts();
  encG = 0;
  encD = 0;
  interrupts();
}

