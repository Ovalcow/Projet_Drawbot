#include "sequences.h"
#include "motors.h"
#include "config.h"

static const unsigned long STEP_PAUSE_MS = 500;

void sequenceEscalier() {
  DBG("[SEQ] S1 escalier debut");

  moveRobotStraight(FORWARD, activeProfile.pwm_straight, activeProfile.dur_20cm);
  delay(STEP_PAUSE_MS);

  turnRobot(TURN_LEFT, activeProfile.pwm_turn, activeProfile.dur_90deg);
  delay(STEP_PAUSE_MS);

  moveRobotStraight(FORWARD, activeProfile.pwm_straight, activeProfile.dur_10cm);
  delay(STEP_PAUSE_MS);

  turnRobot(TURN_RIGHT, activeProfile.pwm_turn, activeProfile.dur_90deg);
  delay(STEP_PAUSE_MS);

  moveRobotStraight(FORWARD, activeProfile.pwm_straight, activeProfile.dur_40cm);

  DBG("[SEQ] S1 escalier fin");
}

void sequenceCircle(int radius_cm) {
  DBG2("[SEQ] S2 cercle rayon cm=", radius_cm);

  // Version simple sans encodeurs : on avance en arc pendant une duree estimee.
  // Le rayon reel depend de l'ecartement des roues, de l'adherence et du sol.
  const int safe_radius = constrain(radius_cm, 8, 80);
  const unsigned long duration_ms = map(safe_radius, 8, 80, 2500, 7000);
  turnRobot(TURN_RIGHT, activeProfile.pwm_turn, duration_ms);

  DBG("[SEQ] S2 cercle fin");
}

void sequenceNorthArrow() {
  DBG("[SEQ] S3 fleche Nord debut");

  // Sans boussole ni gyroscope, "Nord" signifie ici la direction initiale du robot.
  // La sequence trace une tige puis deux petits traits formant une pointe de fleche.
  moveRobotStraight(FORWARD, activeProfile.pwm_straight, activeProfile.dur_40cm);
  delay(STEP_PAUSE_MS);

  turnRobot(TURN_LEFT, activeProfile.pwm_turn, activeProfile.dur_90deg / 2);
  delay(STEP_PAUSE_MS);
  moveRobotStraight(FORWARD, activeProfile.pwm_straight, activeProfile.dur_10cm);
  delay(STEP_PAUSE_MS);

  moveRobotStraight(BACKWARD, activeProfile.pwm_straight, activeProfile.dur_10cm);
  delay(STEP_PAUSE_MS);
  turnRobot(TURN_RIGHT, activeProfile.pwm_turn, activeProfile.dur_90deg);
  delay(STEP_PAUSE_MS);
  moveRobotStraight(FORWARD, activeProfile.pwm_straight, activeProfile.dur_10cm);

  DBG("[SEQ] S3 fleche Nord fin");
}
