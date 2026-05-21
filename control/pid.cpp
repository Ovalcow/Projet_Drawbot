#include "pid.h"
#include "config/config.h"

PIDController::PIDController(float kp, float ki, float kd,
                             float output_min, float output_max)
  : _kp(kp), _ki(ki), _kd(kd),
    _outMin(output_min), _outMax(output_max),
    _integral(0.0f), _lastError(0.0f), _lastOutput(0.0f),
    _firstRun(true)
{}

// ═══════════════════════════════════════════════════════════════
float PIDController::compute(float setpoint, float measurement, float dt)
{
  if (dt <= 0.0f) return _lastOutput;

  float error = setpoint - measurement;

  // Terme proportionnel
  float P = _kp * error;

  // Terme intégral avec anti-windup (clamping)
  _integral += error * dt;
  float iMax = (_outMax - _outMin) / (2.0f * max(_ki, 0.001f));
  _integral = constrain(_integral, -iMax, iMax);
  float I = _ki * _integral;

  // Terme dérivé (sur l'erreur, pas sur la mesure pour simplifier)
  float D = 0.0f;
  if (!_firstRun) {
    D = _kd * (error - _lastError) / dt;
  }
  _firstRun = false;

  // Sortie totale saturée
  float output = constrain(P + I + D, _outMin, _outMax);

  _lastError  = error;
  _lastOutput = output;

  return output;
}

// ═══════════════════════════════════════════════════════════════
void PIDController::reset()
{
  _integral   = 0.0f;
  _lastError  = 0.0f;
  _lastOutput = 0.0f;
  _firstRun   = true;
}

// ═══════════════════════════════════════════════════════════════
void PIDController::setGains(float kp, float ki, float kd)
{
  _kp = kp;
  _ki = ki;
  _kd = kd;
}
