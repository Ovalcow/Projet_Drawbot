#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- Nom du Périphérique Bluetooth ---
#define BLUETOOTH_DEVICE_NAME "Drawbot_ECE_Fusion"

// --- Définitions des Broches (Basées sur CDC p.6) ---
// User LED
#define LEDU1 25 // Optionnel : LED pour voir l'état [cite: 160]

// Enable moteurs droit et gauche
#define EN_G_PIN 4  // Enable Moteur Gauche (Left) [cite: 160]
#define EN_D_PIN 23 // Enable Moteur Droit (Right) [cite: 160]

// Commande Moteur Gauche (Left)
#define IN_1_G_PIN 17 // [cite: 160]
#define IN_2_G_PIN 16 // [cite: 160]

// Commande Moteur Droit (Right)
#define IN_1_D_PIN 18 // [cite: 160]
#define IN_2_D_PIN 19 // [cite: 160]

// Broches I2C pour Magnétomètre
#define SDA_PIN 21 // Broche I2C SDA [cite: 160]
#define SCL_PIN 22 // Broche I2C SCL [cite: 160]

// Adresse I2C LIS3MDL
#define LIS3MDL_I2C_ADDRESS 0x1E // [cite: 160]

// --- Paramètres PWM Moteurs ---
#define PWM_FREQ 5000       // Fréquence PWM (Hz) [cite: 97, 149]
#define PWM_RESOLUTION 8    // Résolution PWM (8 bits = 0-255) [cite: 97]
#define PWM_MAX 255         // Valeur PWM maximale [cite: 100]

// Canaux PWM ESP32
#define PWM_CHANNEL_L_IN1 0
#define PWM_CHANNEL_L_IN2 1
#define PWM_CHANNEL_R_IN1 2
#define PWM_CHANNEL_R_IN2 3

// --- Facteurs de Correction Moteurs (A AJUSTER PAR ESSAIS) ---
#define LEFT_MOTOR_CORRECTION_FACTOR 1.00 // [cite: 101]
#define RIGHT_MOTOR_CORRECTION_FACTOR 1.00 // [cite: 102]

// --- Paramètres Séquence 1 : Escalier (Contrôle par Durée) ---
#define BASE_PWM_STRAIGHT_ESC 220 // Puissance pour avancer/reculer
#define BASE_PWM_TURN_ESC     220 // Puissance pour tourner (arc)
#define DURATION_FWD_20CM     320 // Durée pour 20cm [cite: 125]
#define DURATION_TURN_90DEG_ESC_1 1300 // Durée pour 90 degrés [cite: 126]
#define DURATION_TURN_90DEG_ESC_2 1350 // Durée pour 90 degrés [cite: 126]
#define DURATION_FWD_10CM     0  // Durée pour 10cm (calculé ~ 550/2) [cite: 127]
#define DURATION_FWD_40CM     400 // Durée pour 40cm (calculé ~ 550*2) [cite: 128]
#define INNER_WHEEL_SPEED_RATIO_ARC_TURN 0.45 // Ratio vitesse roue interne [cite: 118, 121]

// --- Paramètres Séquence 3 : Flèche / Boussole ---
// Calibration & Orientation
#define MAGNETIC_DECLINATION_DEG 2.35     // Déclinaison magnétique [cite: 37]
#define HEADING_ACCURACY_DEG 2.5          // Marge d'erreur pour le Nord [cite: 45, 51]
#define CALIBRATION_DURATION_MS 10000     // Durée calibration [cite: 23]
#define CALIBRATION_ROTATE_SPEED_PWM 180  // Vitesse rotation calibration [cite: 23]
#define ORIENTATION_ROTATE_SPEED_PWM 180  // Vitesse rotation orientation [cite: 83]
// Dessin Flèche
#define FORWARD_MOVE_SPEED_PWM 200       // Vitesse avance corps flèche [cite: 84]
#define FORWARD_MOVE_DURATION_MS 150      // Durée avance corps flèche [cite: 84]
// Dessin Pointe Triangle
#define NUM_TRIANGLE_STROKES 2                   // Nombre de traits [cite: 59]
#define TRIANGLE_TURN_SPEED_PWM 170           // Vitesse rotation triangle [cite: 67]
#define TRIANGLE_STROKE_ADVANCE_SPEED_PWM 170   // Vitesse avance segments triangle [cite: 68]
#define TRIANGLE_BASE_ADVANCE_SPEED_PWM 170    // Vitesse avance base triangle [cite: 70]
#define MAX_TRIANGLE_TURN_DURATION_MS 80        // Durée max virage triangle [cite: 60]
#define MIN_TRIANGLE_TURN_DURATION_MS 10        // Durée min virage triangle [cite: 61]
#define TRIANGLE_STROKE_SEGMENT_ADVANCE_DURATION_MS 10 // Durée avance segment [cite: 68]
#define TRIANGLE_INTER_STROKE_ADVANCE_DURATION_MS 30   // Durée avance inter-traits [cite: 70]

// --- Définitions pour la clarté du code ---
#define LEFT_MOTOR 0
#define RIGHT_MOTOR 1
#define FORWARD 1
#define BACKWARD -1
#define STOP 0
#define TURN_LEFT -1
#define TURN_RIGHT 1

#endif // CONFIG_H