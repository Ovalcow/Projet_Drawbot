#include "pid.h"

#include <Arduino.h>

PIDController::PIDController(float kp, float ki, float kd)
    : kp_(kp),
      ki_(ki),
      kd_(kd),
      minOutput_(-255.0f),
      maxOutput_(255.0f),
      integral_(0.0f),
      previousError_(0.0f),
      hasPrevious_(false) {}

void PIDController::setTunings(float kp, float ki, float kd) {
  kp_ = kp;
  ki_ = ki;
  kd_ = kd;
}

void PIDController::setOutputLimits(float minOutput, float maxOutput) {
  minOutput_ = min(minOutput, maxOutput);
  maxOutput_ = max(minOutput, maxOutput);
}

void PIDController::reset() {
  integral_ = 0.0f;
  previousError_ = 0.0f;
  hasPrevious_ = false;
}

float PIDController::compute(float setpoint, float measurement, float dtSeconds) {
  if (dtSeconds <= 0.0f) {
    return 0.0f;
  }

  const float error = setpoint - measurement;
  integral_ += error * dtSeconds;
  const float derivative = hasPrevious_ ? (error - previousError_) / dtSeconds : 0.0f;
  previousError_ = error;
  hasPrevious_ = true;

  const float output = kp_ * error + ki_ * integral_ + kd_ * derivative;
  return constrain(output, minOutput_, maxOutput_);
}
