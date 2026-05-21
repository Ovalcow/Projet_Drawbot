#include "sequences.h"
#include "motors.h"
#include "config.h"

extern RobotProfile activeProfile;

static const unsigned long STEP_PAUSE_MS = 800;

// -------------------------------------------------------
void sequenceEscalier() {
  DBG("[SEQ] Escalier debut");

  moveRobotStraight(FORWARD, activeProfile.pwm_straight, activeProfile.dur_20cm);
  delay(STEP_PAUSE_MS);

  turnRobot(TURN_LEFT, activeProfile.pwm_turn, activeProfile.dur_90deg);
  delay(STEP_PAUSE_MS);

  if (activeProfile.dur_10cm > 0) {
    moveRobotStraight(FORWARD, activeProfile.pwm_straight, activeProfile.dur_10cm);
    delay(STEP_PAUSE_MS);
  }

  turnRobot(TURN_RIGHT, activeProfile.pwm_turn, activeProfile.dur_90deg);
  delay(STEP_PAUSE_MS);

  moveRobotStraight(FORWARD, activeProfile.pwm_straight, activeProfile.dur_40cm);

  DBG("[SEQ] Escalier fin");
}

// -------------------------------------------------------
void sequenceCarre() {
  DBG("[SEQ] Carre debut");

  for (int i = 0; i < 4; i++) {
    moveRobotStraight(FORWARD, activeProfile.pwm_straight, activeProfile.dur_20cm);
    delay(STEP_PAUSE_MS);
    turnRobot(TURN_RIGHT, activeProfile.pwm_turn, activeProfile.dur_90deg);
    delay(STEP_PAUSE_MS);
  }

  DBG("[SEQ] Carre fin");
}

// -------------------------------------------------------
void sequenceZigzag() {
  DBG("[SEQ] Zigzag debut");

  for (int i = 0; i < 3; i++) {
    moveRobotStraight(FORWARD, activeProfile.pwm_straight, activeProfile.dur_20cm);
    delay(STEP_PAUSE_MS);
    int dir = (i % 2 == 0) ? TURN_LEFT : TURN_RIGHT;
    turnRobot(dir, activeProfile.pwm_turn, activeProfile.dur_90deg / 2);
    delay(STEP_PAUSE_MS);
  }
  moveRobotStraight(FORWARD, activeProfile.pwm_straight, activeProfile.dur_20cm);

  DBG("[SEQ] Zigzag fin");
}