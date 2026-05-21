#ifndef MOTORS_H
#define MOTORS_H

#include "config.h"

// Initialise les canaux PWM et les broches moteurs
void setupMotors();

// Commande un moteur individuel
// motor_id : LEFT_MOTOR | RIGHT_MOTOR
// direction : FORWARD | BACKWARD | STOP
// pwm_value : 0-255
void setMotorPower(int motor_id, int direction, int pwm_value);

// Arrête les deux moteurs immédiatement
void stopMotors();

// Avance ou recule en ligne droite pendant duration_ms millisecondes
void moveRobotStraight(int direction, int base_pwm, unsigned long duration_ms);

// Tourne en arc de cercle (les deux roues avancent, la roue intérieure ralentie)
// turn_direction : TURN_LEFT | TURN_RIGHT
void turnRobot(int turn_direction, int base_pwm, unsigned long duration_ms);

#endif // MOTORS_H