#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

#define BLUETOOTH_DEVICE_NAME "Drawbot_PID_Tune"

// LEDs Utilisateur (pour feedback visuel du mode ASSER)
#define LEDU1_PIN 25 //

// Moteurs (CDC Slide 6)
const int MOTEUR_D_EN_PIN = 23;
const int MOTEUR_D_IN1_PIN = 18;
const int MOTEUR_D_IN2_PIN = 19;
const int MOTEUR_G_EN_PIN = 4;
const int MOTEUR_G_IN1_PIN = 17;
const int MOTEUR_G_IN2_PIN = 16;

// Encodeurs (CDC Slide 6)
const int ENCODEUR_G_CHA_PIN = 32;
const int ENCODEUR_G_CHB_PIN = 33;
const int ENCODEUR_D_CHA_PIN = 27;
const int ENCODEUR_D_CHB_PIN = 14;

// Paramètres physiques (non utilisés pour ASSER, mais gardés pour info)
// const float DIAMETRE_ROUE_CM = 9.0f; //
// #ifndef M_PI
//     #define M_PI 3.14159265358979323846f
// #endif
// const float PERIMETRE_ROUE_CM = DIAMETRE_ROUE_CM * M_PI;
// const float TICKS_PAR_TOUR_ROUE = 4220.0f; // Votre mesure

// --- Constantes PID pour l'ASSERVISSEMENT DE POSITION (mode ASSER) ---
// Commencez avec Kp, puis ajoutez Kd, puis Ki.
const double ASSER_DEFAULT_KP = 1.0;  // Valeur de départ faible pour Kp
const double ASSER_DEFAULT_KI = 0.0;
const double ASSER_DEFAULT_KD = 0.0;
const double ASSER_PID_OUTPUT_LIMIT = 200.0; // Puissance max que le PID peut commander (0-255)

#endif // CONFIG_H