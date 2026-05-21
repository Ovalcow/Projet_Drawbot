#ifndef MOTORS_H
#define MOTORS_H

#include "config.h"

// Initialise les sorties moteur et les canaux PWM.
void setupMotors();

// Commande un moteur individuel.
// motor_id  : LEFT_MOTOR ou RIGHT_MOTOR
// direction : FORWARD, BACKWARD ou STOP
// pwm_value : puissance entre 0 et 255
void setMotorPower(int motor_id, int direction, int pwm_value);

// Arrete les deux moteurs immediatement.
void stopMotors();

// Avance ou recule en ligne droite pendant duration_ms millisecondes.
void moveRobotStraight(int direction, int base_pwm, unsigned long duration_ms);

// Tourne en arc de cercle en ralentissant la roue interieure.
// turn_direction : TURN_LEFT ou TURN_RIGHT
void turnRobot(int turn_direction, int base_pwm, unsigned long duration_ms);

#endif // MOTORS_H
