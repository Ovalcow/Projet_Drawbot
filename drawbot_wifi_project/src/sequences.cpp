#include "sequences.h"

#include "config.h"
#include "motors.h"
#include "sensors.h"

namespace {
struct Step {
  MotionKind kind;
  int leftPower;
  int rightPower;
  uint32_t durationMs;
  const char* label;
};

constexpr uint8_t MAX_STEPS = 16;

SequenceKind activeKind = SequenceKind::None;
Step steps[MAX_STEPS];
uint8_t stepCount = 0;
uint8_t currentStep = 0;
bool stepStarted = false;
float activeRadius = 0.0f;

void clearSequence() {
  activeKind = SequenceKind::None;
  stepCount = 0;
  currentStep = 0;
  stepStarted = false;
  activeRadius = 0.0f;
}

uint32_t scaledDuration(uint32_t baseDuration) {
  return clampDuration(baseDuration);
}

bool beginSequence(SequenceKind kind, uint8_t count) {
  if (count == 0 || count > MAX_STEPS) {
    Serial.println("[SEQ][ERREUR] Nombre d'etapes invalide");
    return false;
  }
  stopMotors();
  activeKind = kind;
  stepCount = count;
  currentStep = 0;
  stepStarted = false;
  Serial.printf("[SEQ] Sequence lancee: %s (%u etapes)\n", sequenceKindToString(kind), count);
  return true;
}

void startStep(const Step& step) {
  Serial.printf("[SEQ] Etape %u/%u: %s PWM_G=%d PWM_D=%d duree=%lu ms\n",
                currentStep + 1, stepCount, step.label, step.leftPower, step.rightPower,
                static_cast<unsigned long>(step.durationMs));
  setMotors(step.leftPower, step.rightPower, step.durationMs);
  stepStarted = true;
}

uint32_t distanceDurationMs(float distanceCm) {
  const float ms = (distanceCm / 10.0f) * static_cast<float>(DURATION_10CM_MS);
  return clampDuration(static_cast<uint32_t>(roundf(ms)));
}

void addStep(uint8_t index, MotionKind kind, int leftPower, int rightPower,
             uint32_t durationMs, const char* label) {
  steps[index] = Step{kind, leftPower, rightPower, clampDuration(durationMs), label};
}
}  // namespace

void setupSequences() {
  clearSequence();
}

void updateSequences() {
  if (activeKind == SequenceKind::None) {
    return;
  }

  updateMotors();

  if (!stepStarted) {
    startStep(steps[currentStep]);
    return;
  }

  if (isMotionActive()) {
    return;
  }

  currentStep++;
  stepStarted = false;
  if (currentStep >= stepCount) {
    Serial.printf("[SEQ] Sequence terminee: %s\n", sequenceKindToString(activeKind));
    clearSequence();
  }
}

void stopSequences() {
  if (activeKind != SequenceKind::None) {
    Serial.printf("[SEQ] Arret demande: %s\n", sequenceKindToString(activeKind));
  }
  clearSequence();
  stopMotors();
}

bool startSequenceEscalier() {
  const int straight = DEFAULT_STRAIGHT_PWM;
  const int turn = DEFAULT_TURN_PWM;
  addStep(0, MotionKind::Forward, straight, straight, scaledDuration(DURATION_20CM_MS), "Avancer 20 cm");
  addStep(1, MotionKind::TurnLeft, -turn, turn, scaledDuration(DURATION_TURN_90_MS), "Tourner 90 deg gauche");
  addStep(2, MotionKind::Forward, straight, straight, scaledDuration(DURATION_10CM_MS), "Avancer 10 cm");
  addStep(3, MotionKind::TurnRight, turn, -turn, scaledDuration(DURATION_TURN_90_MS), "Tourner 90 deg droite");
  addStep(4, MotionKind::Forward, straight, straight, scaledDuration(DURATION_40CM_MS), "Avancer 40 cm");
  return beginSequence(SequenceKind::Escalier, 5);
}

bool startSequenceCircle(float radiusCm) {
  const float radius = constrain(radiusCm, CIRCLE_MIN_RADIUS_CM, CIRCLE_MAX_RADIUS_CM);
  activeRadius = radius;

  const float halfBase = WHEEL_BASE_CM / 2.0f;
  const float outerRadius = radius + halfBase;
  const float innerRadius = max(0.5f, radius - halfBase);
  const float ratio = constrain(innerRadius / outerRadius, 0.15f, 1.0f);
  const int outerPwm = CIRCLE_OUTER_PWM;
  const int innerPwm = max(45, static_cast<int>(roundf(static_cast<float>(outerPwm) * ratio)));

  const float straightCmPerMs = 10.0f / static_cast<float>(DURATION_10CM_MS);
  const float outerArcCm = 2.0f * PI * outerRadius;
  const uint32_t durationMs = clampDuration(static_cast<uint32_t>(roundf(outerArcCm / straightCmPerMs)));

  // Cercle vers la gauche: roue droite exterieure, roue gauche interieure.
  addStep(0, MotionKind::Custom, innerPwm, outerPwm, durationMs, "Cercle open-loop");
  Serial.printf("[SEQ] Cercle rayon=%.1f cm, PWM interieur=%d, exterieur=%d, duree=%lu ms\n",
                radius, innerPwm, outerPwm, static_cast<unsigned long>(durationMs));
  return beginSequence(SequenceKind::Circle, 1);
}

bool startSequenceRose() {
  float heading = 0.0f;
  const bool hasHeading = readHeadingDegrees(heading);
  if (!hasHeading) {
    Serial.println("[SEQ] Rose: magnetometre non disponible, fallback fleche sans orientation Nord");
  } else {
    Serial.printf("[SEQ] Rose: cap magnetometre %.1f deg (TODO correction vers Nord)\n", heading);
  }

  const int straight = DEFAULT_STRAIGHT_PWM;
  const int turn = DEFAULT_TURN_PWM;
  addStep(0, MotionKind::Forward, straight, straight, distanceDurationMs(18), "Fleche: hampe");
  addStep(1, MotionKind::Backward, -straight, -straight, distanceDurationMs(7), "Recul centre fleche");
  addStep(2, MotionKind::TurnLeft, -turn, turn, DURATION_TURN_90_MS / 2, "Branche gauche");
  addStep(3, MotionKind::Forward, straight, straight, distanceDurationMs(8), "Trait oblique gauche");
  addStep(4, MotionKind::Backward, -straight, -straight, distanceDurationMs(8), "Retour pointe");
  addStep(5, MotionKind::TurnRight, turn, -turn, DURATION_TURN_90_MS, "Branche droite");
  addStep(6, MotionKind::Forward, straight, straight, distanceDurationMs(8), "Trait oblique droite");
  addStep(7, MotionKind::Backward, -straight, -straight, distanceDurationMs(8), "Retour pointe");
  addStep(8, MotionKind::TurnLeft, -turn, turn, DURATION_TURN_90_MS / 2, "Recentrage");
  return beginSequence(SequenceKind::Rose, 9);
}

bool startDiagnosticMotors() {
  addStep(0, MotionKind::Custom, 160, 0, 1000, "Moteur gauche avant");
  addStep(1, MotionKind::Custom, 0, 0, 250, "Pause");
  addStep(2, MotionKind::Custom, 0, 160, 1000, "Moteur droit avant");
  addStep(3, MotionKind::Custom, 0, 0, 250, "Pause");
  addStep(4, MotionKind::Forward, 160, 160, 1000, "Deux moteurs avant");
  return beginSequence(SequenceKind::TestMotors, 5);
}

bool startDiagnosticPWM() {
  addStep(0, MotionKind::Forward, 120, 120, 900, "Avance PWM 120");
  addStep(1, MotionKind::Forward, 180, 180, 900, "Avance PWM 180");
  addStep(2, MotionKind::Forward, 230, 230, 900, "Avance PWM 230");
  return beginSequence(SequenceKind::TestPWM, 3);
}

bool startDiagnosticDirections() {
  addStep(0, MotionKind::Forward, 160, 160, 700, "Avant");
  addStep(1, MotionKind::Backward, -160, -160, 700, "Arriere");
  addStep(2, MotionKind::TurnLeft, -160, 160, 700, "Gauche");
  addStep(3, MotionKind::TurnRight, 160, -160, 700, "Droite");
  return beginSequence(SequenceKind::TestDirections, 4);
}

bool isSequenceActive() {
  return activeKind != SequenceKind::None;
}

SequenceKind currentSequenceKind() {
  return activeKind;
}

const char* sequenceKindToString(SequenceKind kind) {
  switch (kind) {
    case SequenceKind::Escalier:
      return "ESCALIER";
    case SequenceKind::Circle:
      return "CERCLE";
    case SequenceKind::Rose:
      return "ROSE";
    case SequenceKind::TestMotors:
      return "TEST_MOTORS";
    case SequenceKind::TestPWM:
      return "TEST_PWM";
    case SequenceKind::TestDirections:
      return "TEST_DIR";
    case SequenceKind::None:
    default:
      return "NONE";
  }
}

String sequenceStatusText() {
  if (activeKind == SequenceKind::None) {
    return "Aucune sequence";
  }
  String text = String(sequenceKindToString(activeKind));
  text += " etape ";
  text += String(currentStep + 1);
  text += "/";
  text += String(stepCount);
  if (activeKind == SequenceKind::Circle) {
    text += " rayon ";
    text += String(activeRadius, 1);
    text += " cm";
  }
  return text;
}
