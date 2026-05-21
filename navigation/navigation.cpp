#include "navigation.h"

#include "../drivers/motors.h"
#include "../drivers/encoders.h"
#include "../drivers/imu.h"
#include "../drivers/mag.h"
#include "../control/pid.h"
#include "../config/config.h"


extern RobotProfile activeProfile;

// ── PID instances ──
static PIDController pidSpeedL(PID_SPEED_KP, PID_SPEED_KI, PID_SPEED_KD, -PWM_MAX, PWM_MAX);
static PIDController pidSpeedR(PID_SPEED_KP, PID_SPEED_KI, PID_SPEED_KD, -PWM_MAX, PWM_MAX);
static PIDController pidAngle (PID_ANGLE_KP, PID_ANGLE_KI, PID_ANGLE_KD, -PWM_MAX, PWM_MAX);

// ── Constantes internes ──
static const float    DT           = PID_SAMPLE_MS / 1000.0f;   // en secondes
static const float    DIST_TOL_CM  = 0.3f;   // tolérance distance
static const float    ANGLE_TOL    = 2.0f;   // tolérance angle (°)
static const uint32_t TIMEOUT_MS   = 10000;  // sécurité anti-blocage

// ═══════════════════════════════════════════════════════════════
void setupNavigation()
{
  pidSpeedL.reset();
  pidSpeedR.reset();
  pidAngle.reset();
  DBG("[Nav] OK");
}

// ═══════════════════════════════════════════════════════════════
//  avancerCm — ligne droite asservie en distance (encodeurs)
//  avec correction d'angle par le gyro pour rester droit
// ═══════════════════════════════════════════════════════════════
void avancerCm(float distance_cm)
{
  DBGF("[Nav] avancerCm(%.1f)", distance_cm);

  int dir = (distance_cm >= 0) ? 1 : -1;
  float target = fabsf(distance_cm);

  resetEncoders();
  pidSpeedL.reset();
  pidSpeedR.reset();

  int basePWM = activeProfile.pwm_straight;
  long lastTickL = 0, lastTickR = 0;
  float rpmL_filt = activeProfile.target_rpm * 0.7f;
  float rpmR_filt = activeProfile.target_rpm * 0.7f;
  uint32_t t0 = millis();

  while (true) {
    if (millis() - t0 > TIMEOUT_MS) {
      DBG("[Nav] TIMEOUT avancerCm");
      break;
    }

    float dist = fabsf(getDistanceCm());
    if (dist >= target) break;

    long tickL = getCountLeft();
    long tickR = getCountRight();
    float rpmL = computeRPM(tickL - lastTickL, DT);
    float rpmR = computeRPM(tickR - lastTickR, DT);
    lastTickL = tickL;
    lastTickR = tickR;

    // Filtre passe-bas
    rpmL_filt = 0.85f * rpmL_filt + 0.15f * fabsf(rpmL);
    rpmR_filt = 0.85f * rpmR_filt + 0.15f * fabsf(rpmR);

    // PID vitesse
    float cmdL = pidSpeedL.compute(activeProfile.target_rpm, rpmL_filt, DT);
    float cmdR = pidSpeedR.compute(activeProfile.target_rpm, rpmR_filt, DT);

    // Clamp
    cmdL = constrain(cmdL, -120.0f, 120.0f);
    cmdR = constrain(cmdR, -120.0f, 120.0f);

    // PWM final — pas de gyro
    int pwmL = constrain((int)(basePWM + cmdL) * dir, -PWM_MAX, PWM_MAX);
    int pwmR = constrain((int)(basePWM + cmdR) * dir, -PWM_MAX, PWM_MAX);

    setMotorPWM(LEFT_MOTOR,  pwmL);
    setMotorPWM(RIGHT_MOTOR, pwmR);

    delay(PID_SAMPLE_MS);
  
  }

  stopMotors();
  delay(100);
  DBGF("[Nav] avancerCm done — dist=%.1f cm", fabsf(getDistanceCm()));
}

// ═══════════════════════════════════════════════════════════════
//  tournerDeg — rotation sur place asservie par le gyro (IMU)
//  angle_deg > 0 → droite  |  angle_deg < 0 → gauche
// ═══════════════════════════════════════════════════════════════
void tournerDeg(float angle_deg)
{
  DBGF("[Nav] tournerDeg(%.1f)", angle_deg);

  resetAngle();
  pidAngle.reset();

  int turnDir = (angle_deg >= 0) ? 1 : -1;
  float targetAngle = fabsf(angle_deg);

  uint32_t t0 = millis();

  while (true) {
    if (millis() - t0 > TIMEOUT_MS) {
      DBG("[Nav] TIMEOUT tournerDeg");
      break;
    }

    updateAngle(DT);
    float currentAngle = fabsf(getAngle());

    if (currentAngle >= targetAngle - ANGLE_TOL) break;

    float cmd = pidAngle.compute(targetAngle, currentAngle, DT);
    int pwm = constrain((int)cmd, 0, activeProfile.pwm_turn);

    // Rotation sur place : roues en sens opposé
    setMotorPWM(LEFT_MOTOR,   pwm * turnDir);
    setMotorPWM(RIGHT_MOTOR, -pwm * turnDir);

    delay(PID_SAMPLE_MS);
  }

  stopMotors();
  delay(100);
  DBGF("[Nav] tournerDeg done — angle=%.1f°", fabsf(getAngle()));
}

// ═══════════════════════════════════════════════════════════════
//  tracerCercle — cercle de rayon R (cm), asservissement
//  vitesse différentielle + intégration gyro pour 360°
//
//  v_ext / v_int = (R + L/2) / (R - L/2)
//  où L = entraxe des roues
// ═══════════════════════════════════════════════════════════════
void tracerCercle(float rayon_cm)
{
  DBGF("[Nav] tracerCercle(R=%.1f cm)", rayon_cm);

  if (rayon_cm < 1.0f) {
    DBG("[Nav] Rayon trop petit !");
    return;
  }

  float L = WHEEL_BASE_CM;

  // Rapport de vitesse intérieure / extérieure
  float ratio = (rayon_cm - L / 2.0f) / (rayon_cm + L / 2.0f);
  if (ratio < 0.0f) ratio = 0.0f; // rayon très petit → pivot

  DBGF("[Nav] ratio v_int/v_ext = %.3f", ratio);

  resetAngle();
  pidSpeedL.reset();
  pidSpeedR.reset();

  float targetRPM_ext = activeProfile.target_rpm;
  float targetRPM_int = targetRPM_ext * ratio;

  int basePWM_ext = activeProfile.pwm_straight;
  int basePWM_int = (int)(basePWM_ext * ratio);

  long lastTickL = 0, lastTickR = 0;
  resetEncoders();

  uint32_t t0 = millis();
  // Tourner dans le sens horaire : roue gauche = extérieure
  // Pour le sens anti-horaire, inverser les rôles

  while (true) {
    if (millis() - t0 > 30000) { // timeout 30s pour un cercle
      DBG("[Nav] TIMEOUT tracerCercle");
      break;
    }

    updateAngle(DT);
    float angleTravelled = fabsf(getAngle());

    // Cercle complet = 360°
    if (angleTravelled >= 355.0f) break;

    // Mesure RPM
    long tickL = getCountLeft();
    long tickR = getCountRight();
    float rpmL = computeRPM(tickL - lastTickL, DT);
    float rpmR = computeRPM(tickR - lastTickR, DT);
    lastTickL = tickL;
    lastTickR = tickR;

    // PID vitesse chaque roue
    float cmdL = pidSpeedL.compute(targetRPM_ext, fabsf(rpmL), DT);
    float cmdR = pidSpeedR.compute(targetRPM_int, fabsf(rpmR), DT);

    int pwmL = constrain(basePWM_ext + (int)cmdL, 0, PWM_MAX);
    int pwmR = constrain(basePWM_int + (int)cmdR, 0, PWM_MAX);

    setMotorPWM(LEFT_MOTOR,  pwmL);
    setMotorPWM(RIGHT_MOTOR, pwmR);

    delay(PID_SAMPLE_MS);
  }

  stopMotors();
  delay(100);
  DBGF("[Nav] tracerCercle done — angle total=%.1f°", fabsf(getAngle()));
}

// ═══════════════════════════════════════════════════════════════
//  orienterVersNord — utilise le magnétomètre pour tourner
//  jusqu'à ce que le heading soit ~0° (Nord)
// ═══════════════════════════════════════════════════════════════
void orienterVersNord()
{
  DBG("[Nav] orienterVersNord");

  pidAngle.reset();

  uint32_t t0 = millis();

  while (true) {
    if (millis() - t0 > TIMEOUT_MS) {
      DBG("[Nav] TIMEOUT orienterVersNord");
      break;
    }

    float heading = getHeading(); // 0-360°

    // Erreur signée : combien faut-il tourner pour atteindre 0° ?
    float error = heading;
    if (error > 180.0f) error -= 360.0f; // [-180, 180]

    // On est au Nord ?
    if (fabsf(error) < 5.0f) break;

    float cmd = pidAngle.compute(0.0f, error, DT);
    int pwm = constrain((int)fabsf(cmd), 30, activeProfile.pwm_turn);

    int dir = (error > 0) ? -1 : 1; // heading>0 → trop à l'est → tourner gauche

    setMotorPWM(LEFT_MOTOR,   pwm * dir);
    setMotorPWM(RIGHT_MOTOR, -pwm * dir);

    delay(PID_SAMPLE_MS);
  }

  stopMotors();
  delay(100);
  DBGF("[Nav] orienterVersNord done — heading=%.1f°", getHeading());
}

// ═══════════════════════════════════════════════════════════════
//  Fallback open-loop (pour le joystick / debug)
// ═══════════════════════════════════════════════════════════════
void avancerDuree(int direction, int pwm, unsigned long duration_ms)
{
  setMotorPower(LEFT_MOTOR,  direction, pwm);
  setMotorPower(RIGHT_MOTOR, direction, pwm);
  delay(duration_ms);
  stopMotors();
}

void tournerDuree(int turn_dir, int pwm, unsigned long duration_ms)
{
  int outer_pwm = pwm;
  int inner_pwm = (int)(pwm * activeProfile.inner_wheel_ratio);

  if (turn_dir == TURN_LEFT) {
    setMotorPWM(RIGHT_MOTOR,  outer_pwm);
    setMotorPWM(LEFT_MOTOR,   inner_pwm);
  } else {
    setMotorPWM(LEFT_MOTOR,   outer_pwm);
    setMotorPWM(RIGHT_MOTOR,  inner_pwm);
  }
  delay(duration_ms);
  stopMotors();
}

// ═══════════════════════════════════════════════════════════════
//  Marquer un point (petit trait perpendiculaire ≈ 1cm)
// ═══════════════════════════════════════════════════════════════
void marquerPoint()
{
  // Petit virage gauche + retour pour faire un "T"
  tournerDeg(-15.0f);
  avancerCm(0.5f);
  avancerCm(-0.5f);
  tournerDeg(30.0f);
  avancerCm(0.5f);
  avancerCm(-0.5f);
  tournerDeg(-15.0f);
}
