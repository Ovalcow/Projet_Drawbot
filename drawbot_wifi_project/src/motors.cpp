#include "motors.h"

#include "config.h"

namespace {
struct TimedMotion {
  bool active = false;
  MotionKind kind = MotionKind::Stopped;
  uint32_t startedAtMs = 0;
  uint32_t durationMs = 0;
  int leftPower = 0;
  int rightPower = 0;
};

TimedMotion motion;

int applyCorrection(bool leftMotor, int power) {
  const float correction = leftMotor ? MOTOR_LEFT_CORRECTION : MOTOR_RIGHT_CORRECTION;
  const int corrected = static_cast<int>(roundf(static_cast<float>(power) * correction));
  return constrain(corrected, -static_cast<int>(PWM_MAX), static_cast<int>(PWM_MAX));
}

void writeMotorPins(uint8_t in1Pin, uint8_t in2Pin, int power) {
  const int pwm = abs(constrain(power, -static_cast<int>(PWM_MAX), static_cast<int>(PWM_MAX)));
  if (power > 0) {
    analogWrite(in1Pin, pwm);
    analogWrite(in2Pin, 0);
  } else if (power < 0) {
    analogWrite(in1Pin, 0);
    analogWrite(in2Pin, pwm);
  } else {
    analogWrite(in1Pin, 0);
    analogWrite(in2Pin, 0);
  }
}

void startTimedMotion(MotionKind kind, int leftPower, int rightPower, uint32_t durationMs) {
  const uint32_t boundedDuration = clampDuration(durationMs);
  setMotors(leftPower, rightPower);
  motion.active = boundedDuration > 0;
  motion.kind = motion.active ? kind : MotionKind::Stopped;
  motion.startedAtMs = millis();
  motion.durationMs = boundedDuration;
  motion.leftPower = leftPower;
  motion.rightPower = rightPower;

  Serial.printf("[MOTORS] Mouvement=%s PWM_G=%d PWM_D=%d duree=%lu ms\n",
                motionKindToString(kind), leftPower, rightPower,
                static_cast<unsigned long>(boundedDuration));

  if (!motion.active) {
    stopMotors();
  }
}
}  // namespace

void setupMotors() {
  pinMode(EN_D_PIN, OUTPUT);
  pinMode(EN_G_PIN, OUTPUT);
  pinMode(IN_1_D_PIN, OUTPUT);
  pinMode(IN_2_D_PIN, OUTPUT);
  pinMode(IN_1_G_PIN, OUTPUT);
  pinMode(IN_2_G_PIN, OUTPUT);

  digitalWrite(EN_D_PIN, HIGH);
  digitalWrite(EN_G_PIN, HIGH);
  stopMotors();
  Serial.println("[MOTORS] Initialisation terminee, moteurs arretes");
}

void updateMotors() {
  if (!motion.active) {
    return;
  }

  const uint32_t elapsed = millis() - motion.startedAtMs;
  if (elapsed >= motion.durationMs) {
    Serial.printf("[MOTORS] Fin mouvement=%s apres %lu ms\n",
                  motionKindToString(motion.kind),
                  static_cast<unsigned long>(elapsed));
    stopMotors();
  }
}

void setMotorPower(bool leftMotor, int power) {
  const int correctedPower = applyCorrection(leftMotor, power);
  if (leftMotor) {
    writeMotorPins(IN_1_G_PIN, IN_2_G_PIN, correctedPower);
  } else {
    writeMotorPins(IN_1_D_PIN, IN_2_D_PIN, correctedPower);
  }
}

void setMotors(int leftPower, int rightPower) {
  const int boundedLeft = constrain(leftPower, -static_cast<int>(PWM_MAX), static_cast<int>(PWM_MAX));
  const int boundedRight = constrain(rightPower, -static_cast<int>(PWM_MAX), static_cast<int>(PWM_MAX));
  setMotorPower(true, boundedLeft);
  setMotorPower(false, boundedRight);
  motion.leftPower = boundedLeft;
  motion.rightPower = boundedRight;
}

void setMotors(int leftPower, int rightPower, uint32_t durationMs) {
  startTimedMotion(MotionKind::Custom, leftPower, rightPower, durationMs);
}

void stopMotors() {
  analogWrite(IN_1_D_PIN, 0);
  analogWrite(IN_2_D_PIN, 0);
  analogWrite(IN_1_G_PIN, 0);
  analogWrite(IN_2_G_PIN, 0);
  motion.active = false;
  motion.kind = MotionKind::Stopped;
  motion.durationMs = 0;
  motion.leftPower = 0;
  motion.rightPower = 0;
}

void moveRobotStraight(int pwm, uint32_t durationMs, bool forward) {
  const int boundedPwm = clampPwm(pwm);
  const int signedPwm = forward ? boundedPwm : -boundedPwm;
  startTimedMotion(forward ? MotionKind::Forward : MotionKind::Backward,
                   signedPwm, signedPwm, durationMs);
}

void turnRobot(bool left, int pwm, uint32_t durationMs) {
  const int boundedPwm = clampPwm(pwm);
  const int leftPower = left ? -boundedPwm : boundedPwm;
  const int rightPower = left ? boundedPwm : -boundedPwm;
  startTimedMotion(left ? MotionKind::TurnLeft : MotionKind::TurnRight,
                   leftPower, rightPower, durationMs);
}

void testMotors() {
  Serial.println("[DIAG] TEST_MOTORS bloquant");
  setMotors(160, 0);
  delay(1000);
  stopMotors();
  delay(250);
  setMotors(0, 160);
  delay(1000);
  stopMotors();
  delay(250);
  setMotors(160, 160);
  delay(1000);
  stopMotors();
}

void testPWM() {
  Serial.println("[DIAG] TEST_PWM bloquant");
  const int values[] = {120, 180, 230};
  for (const int pwm : values) {
    Serial.printf("[DIAG] PWM=%d\n", pwm);
    setMotors(pwm, pwm);
    delay(900);
    stopMotors();
    delay(250);
  }
}

void testDirections() {
  Serial.println("[DIAG] TEST_DIR bloquant");
  moveRobotStraight(160, 700, true);
  delay(850);
  moveRobotStraight(160, 700, false);
  delay(850);
  turnRobot(true, 160, 700);
  delay(850);
  turnRobot(false, 160, 700);
  delay(850);
  stopMotors();
}

bool isMotionActive() {
  return motion.active;
}

MotionKind currentMotionKind() {
  return motion.kind;
}

uint32_t currentMotionRemainingMs() {
  if (!motion.active) {
    return 0;
  }
  const uint32_t elapsed = millis() - motion.startedAtMs;
  return elapsed >= motion.durationMs ? 0 : motion.durationMs - elapsed;
}

int currentLeftPower() {
  return motion.leftPower;
}

int currentRightPower() {
  return motion.rightPower;
}

const char* motionKindToString(MotionKind kind) {
  switch (kind) {
    case MotionKind::Forward:
      return "FORWARD";
    case MotionKind::Backward:
      return "BACKWARD";
    case MotionKind::TurnLeft:
      return "TURN_LEFT";
    case MotionKind::TurnRight:
      return "TURN_RIGHT";
    case MotionKind::Custom:
      return "CUSTOM";
    case MotionKind::Stopped:
    default:
      return "STOPPED";
  }
}

int clampPwm(int pwm) {
  return constrain(pwm, static_cast<int>(PWM_MIN), static_cast<int>(PWM_MAX));
}

uint32_t clampDuration(uint32_t durationMs) {
  return min(durationMs, MAX_COMMAND_DURATION_MS);
}
