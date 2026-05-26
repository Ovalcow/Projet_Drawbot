#pragma once

#include <Arduino.h>

enum class SequenceKind : uint8_t {
  None,
  Escalier,
  Circle,
  Rose,
  TestMotors,
  TestPWM,
  TestDirections,
};

void setupSequences();
void updateSequences();
void stopSequences();

bool startSequenceEscalier();
bool startSequenceCircle(float radiusCm);
bool startSequenceRose();
bool startDiagnosticMotors();
bool startDiagnosticPWM();
bool startDiagnosticDirections();

bool isSequenceActive();
SequenceKind currentSequenceKind();
const char* sequenceKindToString(SequenceKind kind);
String sequenceStatusText();
