#pragma once

#include <Arduino.h>

enum class MotionKind : uint8_t {
  Stopped,
  Forward,
  Backward,
  TurnLeft,
  TurnRight,
  Custom,
};

void setupMotors();
void updateMotors();

void setMotorPower(bool leftMotor, int power);
void setMotors(int leftPower, int rightPower);
void setMotors(int leftPower, int rightPower, uint32_t durationMs);
void stopMotors();

void moveRobotStraight(int pwm, uint32_t durationMs, bool forward = true);
void turnRobot(bool left, int pwm, uint32_t durationMs);

void testMotors();
void testPWM();
void testDirections();

bool isMotionActive();
MotionKind currentMotionKind();
uint32_t currentMotionRemainingMs();
int currentLeftPower();
int currentRightPower();
const char* motionKindToString(MotionKind kind);
int clampPwm(int pwm);
uint32_t clampDuration(uint32_t durationMs);
