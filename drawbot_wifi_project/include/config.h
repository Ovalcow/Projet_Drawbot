#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================
//  DEBUG — mettre à 0 pour désactiver les logs Serial verbose
// ============================================================
#define DEBUG_ENABLED 1

#if DEBUG_ENABLED
  #define DBG(msg)     Serial.println(msg)
  #define DBG2(a, b)   { Serial.print(a); Serial.println(b); }
#else
  #define DBG(msg)
  #define DBG2(a, b)
#endif

// ============================================================
//  WIFI ACCESS POINT
//  L'ESP32 crée son propre réseau WiFi.
//  Connecte-toi depuis ton téléphone/PC puis ouvre :
//  http://192.168.4.1  dans le navigateur.
// ============================================================
#define WIFI_AP_SSID      "Drawbot"       // Nom du réseau WiFi visible
#define WIFI_AP_PASSWORD  "drawbot123"    // Mot de passe (min 8 car.) — mettre "" pour réseau ouvert
#define WIFI_AP_IP        "192.168.4.1"    // Adresse habituelle du point d acces ESP32
#define WIFI_AP_CHANNEL   1               // Canal WiFi (1-13)
#define HTTP_PORT         80              // Port du serveur web

// ============================================================
//  BOUTON BOOT & LED INTÉGRÉE
//  Appui sur BOOT (GPIO0) : active / coupe le WiFi
//  LED fixe     = WiFi actif   (robot accessible)
//  LED clignotante = WiFi inactif
// ============================================================
#define WIFI_TOGGLE_PIN   0   // GPIO 0 = bouton BOOT (actif à LOW)
#define WIFI_LED_PIN      2   // GPIO 2 = LED bleue intégrée

// ============================================================
//  BROCHES MOTEURS
// ============================================================
#define EN_G_PIN    4
#define EN_D_PIN   23
#define IN_1_G_PIN 17
#define IN_2_G_PIN 16
#define IN_1_D_PIN 19
#define IN_2_D_PIN 18

// ============================================================
//  ENCODEURS (quadrature)
// ============================================================
// Gauche
#define ENC_G_CH_A 32
#define ENC_G_CH_B 33
// Droite
#define ENC_D_CH_A 27
#define ENC_D_CH_B 14

// Conversion mécanique : ticks par tour (A FIXER après test)
// TODO: Fais tourner une roue ~1 tour mécanique puis relève les ticks
// mesurés sur Serial (getEncG/getEncD).
#ifndef ENC_TICKS_PER_REV
#define ENC_TICKS_PER_REV 20

#endif


// ============================================================
//  PWM
// ============================================================
#define PWM_FREQ          5000
#define PWM_RESOLUTION    8
#define PWM_MAX           255
#define PWM_CHANNEL_L_IN1 0
#define PWM_CHANNEL_L_IN2 1
#define PWM_CHANNEL_R_IN1 2
#define PWM_CHANNEL_R_IN2 3

// ============================================================
//  CONSTANTES LOGIQUES
// ============================================================
#define LEFT_MOTOR  0
#define RIGHT_MOTOR 1
#define FORWARD     1
#define BACKWARD   -1
#define STOP        0
#define TURN_LEFT  -1
#define TURN_RIGHT  1

// ============================================================
//  PROFILS DE RÉGLAGES (sélectionnables depuis l'interface web)
// ============================================================
struct RobotProfile {
  const char*   name;
  int           pwm_straight;
  int           pwm_turn;
  float         left_correction;
  float         right_correction;
  float         inner_wheel_ratio;
  unsigned long dur_20cm;
  unsigned long dur_10cm;
  unsigned long dur_40cm;
  unsigned long dur_90deg;
};

// Profil 0 — Lent / précis
// Profil 1 — Normal (valeurs d'origine)
// Profil 2 — Rapide
static const RobotProfile PROFILES[] = {
  { "Lent",   130, 140, 1.0f, 0.94f, 0.45f,  750, 375,  900, 1900 },
  { "Normal", 190, 200, 1.0f, 0.94f, 0.45f,  550, 375,  600, 1450 },
  { "Rapide", 230, 240, 1.0f, 0.94f, 0.40f,  400, 375,  450, 1100 },
};

#define PROFILE_COUNT         3
#define DEFAULT_PROFILE_INDEX 1   // 0=Lent, 1=Normal, 2=Rapide

#endif // CONFIG_H
