#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================
//  DEBUG — mettre à 0 pour désactiver les logs Serial
// ============================================================
#define DEBUG_ENABLED 1

#if DEBUG_ENABLED
  #define DBG(msg)       Serial.println(msg)
  #define DBG2(a, b)     { Serial.print(a); Serial.println(b); }
  #define DBGF(...)      { Serial.printf(__VA_ARGS__); Serial.println(); }
#else
  #define DBG(msg)
  #define DBG2(a, b)
  #define DBGF(...)
#endif

// ============================================================
//  WIFI ACCESS POINT
// ============================================================
#define WIFI_AP_SSID      "Drawbot"
#define WIFI_AP_PASSWORD  "drawing123"
#define WIFI_AP_IP        "192.168.4.1"
#define WIFI_AP_CHANNEL   1
#define HTTP_PORT         80

// ============================================================
//  BOUTON BOOT & LED
// ============================================================
#define WIFI_TOGGLE_PIN   0   // GPIO 0 = bouton BOOT
#define WIFI_LED_PIN      2   // GPIO 2 = LED bleue intégrée

// ============================================================
//  USER LEDs (sur le PCB Gyrobot)
// ============================================================
#define LEDU1             25
#define LEDU2             26

// ============================================================
//  BROCHES MOTEURS (drivers DRV8837)
// ============================================================
#define EN_G_PIN    4
#define EN_D_PIN   23
#define IN_1_G_PIN 17
#define IN_2_G_PIN 16
#define IN_1_D_PIN 19
#define IN_2_D_PIN 18

// ============================================================
//  PWM ESP32 — canaux LEDC
// ============================================================
#define PWM_FREQ          5000
#define PWM_RESOLUTION    8
#define PWM_MAX           255

#define PWM_CH_L_IN1  0
#define PWM_CH_L_IN2  1
#define PWM_CH_R_IN1  2
#define PWM_CH_R_IN2  3

// ============================================================
//  ENCODEURS (effet Hall, 2 canaux quadrature)
// ============================================================
#define ENC_G_CH_A  32
#define ENC_G_CH_B  33
#define ENC_D_CH_A  27
#define ENC_D_CH_B  14

// Résolution de l'encodeur (impulsions par tour de roue)
#define PULSES_PER_REV   1027.0f

// Géométrie du robot
#define WHEEL_DIAMETER_MM   90.0f                               // diamètre roue en mm
#define WHEEL_CIRCUMFERENCE (WHEEL_DIAMETER_MM * PI / 10.0f)    // en cm
#define WHEEL_BASE_CM       10.0f                               // entraxe roues en cm (à calibrer)

// ============================================================
//  I2C — IMU & Magnétomètre
// ============================================================
#define I2C_SDA_PIN   21
#define I2C_SCL_PIN   22
#define ADDR_IMU      0x6B    // LSM6DS3
#define ADDR_MAG      0x1E    // LIS3MDL

// ============================================================
//  PID — Gains par défaut (à affiner par calibration)
// ============================================================

// PID vitesse (une instance par roue)
#define PID_SPEED_KP    0.8f
#define PID_SPEED_KI    0.25f
#define PID_SPEED_KD    0.3f

// PID angle (virage / orientation)
#define PID_ANGLE_KP    3.0f
#define PID_ANGLE_KI    0.05f
#define PID_ANGLE_KD    0.3f

// Période d'échantillonnage PID (ms)
#define PID_SAMPLE_MS   20

// ============================================================
//  CONSTANTES LOGIQUES
// ============================================================
#define LEFT_MOTOR   0
#define RIGHT_MOTOR  1
#define FORWARD      1
#define BACKWARD    -1
#define STOP_DIR     0
#define TURN_LEFT   -1
#define TURN_RIGHT   1

// ============================================================
//  PROFILS DE RÉGLAGES (vitesse de base, sélectionnables WiFi)
// ============================================================
struct RobotProfile {
  const char*   name;
  int           pwm_straight;     // PWM de base ligne droite
  int           pwm_turn;         // PWM de base virage
  float         left_correction;  // facteur correctif moteur gauche
  float         right_correction; // facteur correctif moteur droit
  float         inner_wheel_ratio;// ratio roue intérieure en virage
  float         target_rpm;       // consigne RPM pour le PID vitesse
};

static const RobotProfile PROFILES[] = {
  //  name      pwm_str  pwm_trn  l_cor   r_cor   inner   rpm
  { "Lent",     90,      140,     0.88f,   1.0f,  0.45f,  40.0f },
  { "Normal",   90,      200,     0.88f,   1.0f,  0.45f,  70.0f },
  { "Rapide",   90,      240,     0.88f,   1.0f,  0.40f,  100.0f },
};

#define PROFILE_COUNT         3
#define DEFAULT_PROFILE_INDEX 1

#endif // CONFIG_H
