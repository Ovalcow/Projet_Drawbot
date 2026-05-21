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
    // Logique inversée côté droit
    if      (direction == FORWARD)  { ledcWrite(PWM_CHANNEL_R_IN1, 0);   ledcWrite(PWM_CHANNEL_R_IN2, pwm); }
    else if (direction == BACKWARD) { ledcWrite(PWM_CHANNEL_R_IN1, pwm); ledcWrite(PWM_CHANNEL_R_IN2, 0);   }
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