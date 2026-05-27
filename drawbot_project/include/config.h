#pragma once

#include <Arduino.h>

// Drawbot - pins imposes par le PDF PRJ_SB_v2026_v2.
constexpr uint8_t LEDU1_PIN = 25;
constexpr uint8_t LEDU2_PIN = 26;

constexpr uint8_t EN_D_PIN = 23;
constexpr uint8_t EN_G_PIN = 4;
constexpr uint8_t IN_1_D_PIN = 19;
constexpr uint8_t IN_2_D_PIN = 18;
constexpr uint8_t IN_1_G_PIN = 17;
constexpr uint8_t IN_2_G_PIN = 16;

constexpr uint8_t ENC_G_CH_A_PIN = 32;
constexpr uint8_t ENC_G_CH_B_PIN = 33;
constexpr uint8_t ENC_D_CH_A_PIN = 27;
constexpr uint8_t ENC_D_CH_B_PIN = 14;

constexpr uint8_t I2C_SDA_PIN = 21;
constexpr uint8_t I2C_SCL_PIN = 22;
constexpr uint8_t ADDR_IMU = 0x6B;
constexpr uint8_t ADDR_MAG = 0x1E;

constexpr uint32_t SERIAL_BAUDRATE = 115200;

constexpr char WIFI_AP_SSID[] = "Drawbot";
constexpr char WIFI_AP_PASSWORD[] = "drawbot123";

// Securite commande.
constexpr uint8_t PWM_MIN = 0;
constexpr uint8_t PWM_MAX = 255;
constexpr uint32_t MAX_COMMAND_DURATION_MS = 15000;
constexpr uint32_t DEFAULT_MANUAL_DURATION_MS = 350;

// Calibration mecanique. A ajuster sur le robot reel.
constexpr float WHEEL_DIAMETER_CM = 9.0f;
constexpr float WHEEL_BASE_CM = 12.0f;       // Entraxe roues, a mesurer.
constexpr float MOTOR_LEFT_CORRECTION = 1.00f;
constexpr float MOTOR_RIGHT_CORRECTION = 1.00f;

// Corrections de sens moteur (signe). A mettre a true si le moteur est inverse cote cablage.
// Par defaut on considere que +PWM correspond a "avant" pour chaque moteur.
constexpr bool INVERT_LEFT_MOTOR = true;
constexpr bool INVERT_RIGHT_MOTOR = true;

// Permet de swapper gauche/droite si les moteurs (canaux) sont connectes en echange.
// Mets a true si, par exemple, "PWM gauche" fait tourner le robot a la place comme si c'etait la droite.
constexpr bool SWAP_LEFT_RIGHT_MOTORS = true;


// Calibration open-loop par defaut.
constexpr uint8_t DEFAULT_STRAIGHT_PWM = 170;
constexpr uint8_t DEFAULT_TURN_PWM = 165;
constexpr uint32_t DURATION_10CM_MS = 900;
constexpr uint32_t DURATION_20CM_MS = 1800;
constexpr uint32_t DURATION_40CM_MS = 3600;
constexpr uint32_t DURATION_TURN_90_MS = 760;

// Cercle open-loop. Le rayon est le rayon du centre du robot.
constexpr float CIRCLE_MIN_RADIUS_CM = 2.0f;
constexpr float CIRCLE_MAX_RADIUS_CM = 20.0f;
constexpr uint8_t CIRCLE_OUTER_PWM = 170;

enum class SpeedProfile : uint8_t {
  Slow = 0,
  Normal = 1,
  Fast = 2,
};

struct ProfileSettings {
  const char* name;
  uint8_t straightPwm;
  uint8_t turnPwm;
  float durationScale;
};

constexpr ProfileSettings SPEED_PROFILES[] = {
  {"Lent", 120, 120, 1.35f},
  {"Normal", DEFAULT_STRAIGHT_PWM, DEFAULT_TURN_PWM, 1.00f},
  {"Rapide", 220, 215, 0.75f},
};

constexpr uint8_t SPEED_PROFILE_COUNT = sizeof(SPEED_PROFILES) / sizeof(SPEED_PROFILES[0]);
