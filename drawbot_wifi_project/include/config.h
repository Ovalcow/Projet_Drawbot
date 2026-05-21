#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================
//  DEBUG - mettre a 0 pour desactiver les logs Serial verbose
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
//  L'ESP32 cree son propre reseau WiFi.
//  Connecte-toi au reseau Drawbot puis ouvre :
//  http://192.168.4.1
// ============================================================
#define WIFI_AP_SSID      "Drawbot"
#define WIFI_AP_PASSWORD  "drawbot123"
#define WIFI_AP_IP        "192.168.4.1"
#define WIFI_AP_GATEWAY   "192.168.4.1"
#define WIFI_AP_SUBNET    "255.255.255.0"
#define WIFI_AP_CHANNEL   1
#define HTTP_PORT         80

// ============================================================
//  BOUTON BOOT & LED INTEGREE
//  BOOT (GPIO0) permet de couper ou relancer le WiFi.
//  LED fixe = WiFi actif, LED clignotante = WiFi inactif.
// ============================================================
#define WIFI_TOGGLE_PIN   0
#define WIFI_LED_PIN      2

// ============================================================
//  BROCHES MOTEURS
//  Pins projet : EN_G=4, EN_D=23, IN_1_G=17, IN_2_G=16,
//  IN_1_D=19, IN_2_D=18.
// ============================================================
#define EN_G_PIN    4
#define EN_D_PIN    23
#define IN_1_G_PIN  17
#define IN_2_G_PIN  16
#define IN_1_D_PIN  19
#define IN_2_D_PIN  18

// ============================================================
//  PWM ESP32
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
//  PROFILS DE REGLAGES
//  Les durees sont a calibrer sur le robot reel.
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

static const RobotProfile PROFILES[] = {
  { "Lent",   130, 140, 1.0f, 0.94f, 0.45f, 750, 375, 1500, 1900 },
  { "Normal", 190, 200, 1.0f, 0.94f, 0.45f, 550, 275, 1100, 1450 },
  { "Rapide", 230, 240, 1.0f, 0.94f, 0.40f, 400, 200,  800, 1100 },
};

#define PROFILE_COUNT         3
#define DEFAULT_PROFILE_INDEX 1

// Profil actif partage par les modules moteurs, sequences et serveur web.
extern RobotProfile activeProfile;

#endif // CONFIG_H
