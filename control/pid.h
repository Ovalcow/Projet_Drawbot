#ifndef PID_H
#define PID_H

#include "config/config.h"

// ═══════════════════════════════════════════════════════════════
//  Correcteur PID numérique — classe générique réutilisable
//
//  Utilisation :
//    PIDController pid(Kp, Ki, Kd, outMin, outMax);
//    pid.reset();
//    float cmd = pid.compute(setpoint, measurement, dt);
// ═══════════════════════════════════════════════════════════════
class PIDController
{
public:
  PIDController(float kp, float ki, float kd,
                float output_min, float output_max);

  // Calcule la sortie PID
  // setpoint   : consigne
  // measurement: mesure actuelle
  // dt         : pas de temps en secondes
  float compute(float setpoint, float measurement, float dt);

  // Remet à zéro l'intégrale et l'erreur précédente
  void reset();

  // Modifier les gains à la volée
  void setGains(float kp, float ki, float kd);

  // Accès lecture
  float getError()    const { return _lastError; }
  float getIntegral() const { return _integral; }
  float getOutput()   const { return _lastOutput; }

private:
  float _kp, _ki, _kd;
  float _outMin, _outMax;
  float _integral;
  float _lastError;
  float _lastOutput;
  bool  _firstRun;
};

#endif // PID_H
