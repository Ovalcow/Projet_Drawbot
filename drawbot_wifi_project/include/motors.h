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

// Tourne sur place (pivot) : une roue en avant, l'autre en arrière
// turn_direction : TURN_LEFT | TURN_RIGHT
void turnRobotPivot(int turn_direction, int base_pwm, unsigned long duration_ms);

// -------------------------------------------------------
//  Encodeurs + fermeture boucle (Phase 2.1)
// -------------------------------------------------------
long getEncG();
long getEncD();

// Avance/recul d'une distance donnée (cm) en boucle fermée
void moveDistance_cm(float dist_cm);

// Rotation sur place d'un angle donné (degrés) en boucle fermée
void rotateDeg(float deg);

#endif // MOTORS_H

