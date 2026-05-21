#include "motors.h"

extern RobotProfile activeProfile;

// -------------------------------------------------------
void setupMotors() {
  ledcSetup(PWM_CHANNEL_L_IN1, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_L_IN2, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_R_IN1, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_R_IN2, PWM_FREQ, PWM_RESOLUTION);

  ledcAttachPin(IN_1_G_PIN, PWM_CHANNEL_L_IN1);
  ledcAttachPin(IN_2_G_PIN, PWM_CHANNEL_L_IN2);
  ledcAttachPin(IN_1_D_PIN, PWM_CHANNEL_R_IN1);
  ledcAttachPin(IN_2_D_PIN, PWM_CHANNEL_R_IN2);

  pinMode(EN_G_PIN, OUTPUT);  digitalWrite(EN_G_PIN, HIGH);
  pinMode(EN_D_PIN, OUTPUT);  digitalWrite(EN_D_PIN, HIGH);

  DBG("[Motors] Initialisés.");
}

// -------------------------------------------------------
void setMotorPower(int motor_id, int direction, int pwm_value) {
  pwm_value = constrain(pwm_value, 0, PWM_MAX);

  float factor = (motor_id == LEFT_MOTOR)
    ? activeProfile.left_correction
    : activeProfile.right_correction;

  int pwm = constrain((int)(pwm_value * factor), 0, PWM_MAX);

  if (motor_id == LEFT_MOTOR) {
    digitalWrite(EN_G_PIN, HIGH);
    if      (direction == FORWARD)  { ledcWrite(PWM_CHANNEL_L_IN1, pwm); ledcWrite(PWM_CHANNEL_L_IN2, 0);   }
    else if (direction == BACKWARD) { ledcWrite(PWM_CHANNEL_L_IN1, 0);   ledcWrite(PWM_CHANNEL_L_IN2, pwm); }
    else                            { ledcWrite(PWM_CHANNEL_L_IN1, 0);   ledcWrite(PWM_CHANNEL_L_IN2, 0);   }

  } else {
    digitalWrite(EN_D_PIN, HIGH);
    // Convention unifiée : FORWARD=IN1 actif, BACKWARD=IN2 actif
    if      (direction == FORWARD)  { ledcWrite(PWM_CHANNEL_R_IN1, pwm); ledcWrite(PWM_CHANNEL_R_IN2, 0);   }
    else if (direction == BACKWARD) { ledcWrite(PWM_CHANNEL_R_IN1, 0);   ledcWrite(PWM_CHANNEL_R_IN2, pwm); }
    else                            { ledcWrite(PWM_CHANNEL_R_IN1, 0);   ledcWrite(PWM_CHANNEL_R_IN2, 0);   }
  }

}

// -------------------------------------------------------
void stopMotors() {
  setMotorPower(LEFT_MOTOR,  STOP, 0);
  setMotorPower(RIGHT_MOTOR, STOP, 0);
  DBG("[Motors] Stop.");
}

// -------------------------------------------------------
void moveRobotStraight(int direction, int base_pwm, unsigned long duration_ms) {
  DBG2("[Motors] Ligne droite | dir=", direction == FORWARD ? "AV" : "AR");
  setMotorPower(LEFT_MOTOR,  direction, base_pwm);
  setMotorPower(RIGHT_MOTOR, direction, base_pwm);
  delay(duration_ms);
  stopMotors();
}

// -------------------------------------------------------
void turnRobot(int turn_direction, int base_pwm, unsigned long duration_ms) {
  int pwm_outer = base_pwm;
  int pwm_inner = constrain(
    (int)(base_pwm * activeProfile.inner_wheel_ratio), 0, PWM_MAX
  );

  DBG2("[Motors] Arc | ratio=", activeProfile.inner_wheel_ratio);

  if (turn_direction == TURN_LEFT) {
    // Gauche : roue droite = extérieure (rapide), gauche = intérieure (lente)
    setMotorPower(RIGHT_MOTOR, FORWARD, pwm_outer);
    setMotorPower(LEFT_MOTOR,  FORWARD, pwm_inner);
  } else {
    // Droite : roue gauche = extérieure (rapide), droite = intérieure (lente)
    setMotorPower(LEFT_MOTOR,  FORWARD, pwm_outer);
    setMotorPower(RIGHT_MOTOR, FORWARD, pwm_inner);
  }
  delay(duration_ms);
  stopMotors();
}

// -------------------------------------------------------
void turnRobotPivot(int turn_direction, int base_pwm, unsigned long duration_ms) {
  // Pivot sur place : une roue avant, l'autre arrière
  // Convention : TURN_LEFT => robot pivote vers la gauche
  if (turn_direction == TURN_LEFT) {
    setMotorPower(LEFT_MOTOR,  FORWARD,  base_pwm);
    setMotorPower(RIGHT_MOTOR, BACKWARD, base_pwm);
  } else {
    setMotorPower(LEFT_MOTOR,  BACKWARD, base_pwm);
    setMotorPower(RIGHT_MOTOR, FORWARD,  base_pwm);
  }

  delay(duration_ms);
  stopMotors();
}

// ============================================================
//  Phase 2.1 — Boucle ouverte en ticks (sans PID)
// ============================================================
// NOTE: les conversions cm/deg -> ticks sont volontairement des placeholders
// pour valider le pipeline (encodeurs -> boucles -> arrêt). À ajuster lors
// du calibrage mécanique.

static inline long cmToTicks(float dist_cm) {
  // Placeholder : 1 cm => 1 tick
  return (long)llround(dist_cm);
}

static inline long degToTicks(float deg) {
  // Placeholder : 1 deg => 1 tick
  return (long)llround(deg);
}

static void driveTicks(long ticksG, long ticksD, int base_pwm) {
  long startG = getEncG();
  long startD = getEncD();

  long remG = llabs(ticksG);
  long remD = llabs(ticksD);

  int dirG = (ticksG >= 0) ? FORWARD : BACKWARD;
  int dirD = (ticksD >= 0) ? FORWARD : BACKWARD;

  setMotorPower(LEFT_MOTOR, dirG, base_pwm);
  setMotorPower(RIGHT_MOTOR, dirD, base_pwm);

  // Stop dès que les deux roues ont atteint au moins leurs cibles
  unsigned long startMs = millis();
  const unsigned long timeoutMs = 15000; // sécurité

  while (true) {
    long curG = getEncG() - startG;
    long curD = getEncD() - startD;

    bool okG = llabs(curG) >= remG;
    bool okD = llabs(curD) >= remD;

    if (okG && okD) break;
    if (millis() - startMs > timeoutMs) break;
    delay(5);
  }

  stopMotors();
}

void moveDistance_cm(float dist_cm) {
  int base_pwm = activeProfile.pwm_straight;
  long ticks = cmToTicks(dist_cm);
  DBG2("[MOVE] dist_cm=", dist_cm);
  driveTicks(ticks, ticks, base_pwm);
}

void rotateDeg(float deg) {
  int base_pwm = activeProfile.pwm_turn;
  long ticks = degToTicks(deg);
  DBG2("[ROT] deg=", deg);

  // Convention: rotation positive = TURN_RIGHT (pivot sur place)
  if (ticks >= 0) {
    // gauche avant, droite arrière
    driveTicks(+ticks, -ticks, base_pwm);
  } else {
    // gauche arrière, droite avant
    driveTicks(ticks, -ticks, base_pwm);
  }
}


