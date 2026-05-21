#ifndef ENCODERS_H
#define ENCODERS_H

#include <Arduino.h>

// Initialise les broches encodeurs + attache les ISR
void setupEncoders();

// Lire compteurs (ticks) de chaque roue
long getEncG();
long getEncD();

// Remet les compteurs à 0
void resetEncoders();

#endif // ENCODERS_H

