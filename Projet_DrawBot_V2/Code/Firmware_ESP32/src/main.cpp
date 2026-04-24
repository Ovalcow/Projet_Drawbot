#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_LIS3MDL.h>
#include <Adafruit_Sensor.h>
#include <BluetoothSerial.h>
#include "config.h"

// --- Objets Globaux ---
Adafruit_LIS3MDL lis3mdl; // [cite: 1]
BluetoothSerial SerialBT; // [cite: 2]

// --- Variables Globales ---
// Pour Boussole
float mag_offset_x = 0.0f; // [cite: 2]
float mag_offset_y = 0.0f; // [cite: 2]
float mag_offset_z = 0.0f; // [cite: 2]
// Pour Contrôle Séquences
volatile bool sequence_running = false; // [cite: 3]
volatile bool sequence_escalier_requested = false;
volatile bool sequence_fleche_requested = false;

// --- Déclarations (Prototypes) ---
// Initialisation
void setupMotors();
void setupBluetooth();
void setupLIS3MDL();
// Contrôle Moteur Unifié
void setMotorPower(int motor_id, int direction, int pwm_value);
void stopMotors();
// Mouvements Basiques
void moveRobotStraight(int direction, int base_pwm_power, unsigned long duration_ms);
void turnRobotPivot(bool clockwise, int speed_pwm);
void turnRobotArc(int turn_direction, int base_pwm_power, unsigned long duration_ms);
void turnRobotTimed(bool clockwise, int speed_pwm, unsigned long duration_ms);
void moveForwardTimed(int speed_pwm, unsigned long duration_ms);
// Fonctions Magnétomètre
void calibrateMagnetometer();
float calculateTrueNorthHeading();
void orientNorth(int speed_pwm);
// Fonctions Dessin
void drawTrianglePoint();
// Séquences
void sequenceEscalier();
void sequenceFleche();
// Communication
void handleBluetoothCommands();

// --- Fonctions d'Initialisation ---
void setupMotors() {
    Serial.print("Configuration Moteurs... ");
    // Pins Moteur Gauche
    pinMode(EN_G_PIN, OUTPUT); // [cite: 99]
    pinMode(IN_1_G_PIN, OUTPUT); // [cite: 9]
    pinMode(IN_2_G_PIN, OUTPUT); // [cite: 9]
    digitalWrite(EN_G_PIN, HIGH); // [cite: 9]
    // Pins Moteur Droit
    pinMode(EN_D_PIN, OUTPUT); // [cite: 9]
    pinMode(IN_1_D_PIN, OUTPUT); // [cite: 9]
    pinMode(IN_2_D_PIN, OUTPUT); // [cite: 9]
    digitalWrite(EN_D_PIN, HIGH); // [cite: 9]
    // Setup PWM Gauche
    ledcSetup(PWM_CHANNEL_L_IN1, PWM_FREQ, PWM_RESOLUTION); // [cite: 97]
    ledcSetup(PWM_CHANNEL_L_IN2, PWM_FREQ, PWM_RESOLUTION); // [cite: 98]
    ledcAttachPin(IN_1_G_PIN, PWM_CHANNEL_L_IN1); // [cite: 98]
    ledcAttachPin(IN_2_G_PIN, PWM_CHANNEL_L_IN2); // [cite: 98]
    // Setup PWM Droit
    ledcSetup(PWM_CHANNEL_R_IN1, PWM_FREQ, PWM_RESOLUTION); // [cite: 98]
    ledcSetup(PWM_CHANNEL_R_IN2, PWM_FREQ, PWM_RESOLUTION); // [cite: 98]
    ledcAttachPin(IN_1_D_PIN, PWM_CHANNEL_R_IN1); // [cite: 98]
    ledcAttachPin(IN_2_D_PIN, PWM_CHANNEL_R_IN2); // [cite: 98]
    stopMotors();
    Serial.println("Moteurs OK.");
}

void setupBluetooth() {
    Serial.print("Initialisation Bluetooth... ");
    if (!SerialBT.begin(BLUETOOTH_DEVICE_NAME)) { // [cite: 12]
        Serial.println("Erreur BT!");
    } else {
        Serial.print("BT OK: "); Serial.println(BLUETOOTH_DEVICE_NAME); // [cite: 12]
        SerialBT.println("Drawbot ECE Pret !"); // [cite: 12]
        SerialBT.println("Envoyez '1' pour Escalier."); // [cite: 130]
        SerialBT.println("Envoyez '3' pour Fleche Nord."); // [cite: 12]
    }
}

void setupLIS3MDL() {
    Serial.print("Initialisation LIS3MDL... ");
    if (!lis3mdl.begin_I2C(LIS3MDL_I2C_ADDRESS, &Wire)) { // [cite: 7]
        Serial.println("Erreur: LIS3MDL non détecté!");
        SerialBT.println("Erreur LIS3MDL!");
        while (1) delay(10);
    }
    Serial.println("LIS3MDL OK."); // [cite: 8]
}

// --- Contrôle Moteur Unifié ---
void setMotorPower(int motor_id, int direction, int pwm_value) {
    pwm_value = constrain(pwm_value, 0, PWM_MAX); // [cite: 100]
    float correction_factor = 1.0;

    if (motor_id == LEFT_MOTOR) {
        correction_factor = LEFT_MOTOR_CORRECTION_FACTOR; // [cite: 101]
    } else if (motor_id == RIGHT_MOTOR) {
        correction_factor = RIGHT_MOTOR_CORRECTION_FACTOR; // [cite: 102]
    }
    int corrected_pwm = (int)(pwm_value * correction_factor);
    corrected_pwm = constrain(corrected_pwm, 0, PWM_MAX); // [cite: 103]

    if (motor_id == LEFT_MOTOR) { // [cite: 106]
        digitalWrite(EN_G_PIN, HIGH); // [cite: 106]
        if (direction == FORWARD) {
            ledcWrite(PWM_CHANNEL_L_IN1, corrected_pwm); ledcWrite(PWM_CHANNEL_L_IN2, 0); // [cite: 107]
        } else if (direction == BACKWARD) {
            ledcWrite(PWM_CHANNEL_L_IN1, 0); ledcWrite(PWM_CHANNEL_L_IN2, corrected_pwm); // [cite: 108]
        } else { // STOP
            ledcWrite(PWM_CHANNEL_L_IN1, 0); ledcWrite(PWM_CHANNEL_L_IN2, 0); // [cite: 109]
        }
    } else if (motor_id == RIGHT_MOTOR) { // [cite: 110]
        digitalWrite(EN_D_PIN, HIGH); // [cite: 110]
        // Utilise IN_1_D_PIN (19) et IN_2_D_PIN (18)
        // Pour AVANCER, le code escalier faisait IN1=0, IN2=PWM.
        // Si IN_1_D_PIN = 19 et IN_2_D_PIN = 18.
        // On vérifie la logique. Si FORWARD = IN2=PWM, IN1=0
        if (direction == FORWARD) {
             ledcWrite(PWM_CHANNEL_R_IN1, corrected_pwm); ledcWrite(PWM_CHANNEL_R_IN2, 0); // Adaptation: IN1=PWM, IN2=0 pour FORWARD
        } else if (direction == BACKWARD) {
             ledcWrite(PWM_CHANNEL_R_IN1, 0); ledcWrite(PWM_CHANNEL_R_IN2, corrected_pwm); // Adaptation: IN1=0, IN2=PWM pour BACKWARD
        } else { // STOP
            ledcWrite(PWM_CHANNEL_R_IN1, 0); ledcWrite(PWM_CHANNEL_R_IN2, 0); // [cite: 113]
        }
    }
}


void stopMotors() {
    setMotorPower(LEFT_MOTOR, STOP, 0); // [cite: 114]
    setMotorPower(RIGHT_MOTOR, STOP, 0); // [cite: 115]
    // Serial.println(">>> MOTEURS ARRETES <<<"); // Optionnel
}

// --- Mouvements Basiques ---
void moveRobotStraight(int direction, int base_pwm_power, unsigned long duration_ms) {
    Serial.print("Avance Droit -> Dir: "); // [cite: 115]
    Serial.print(direction == FORWARD ? "AVANT" : "ARRIERE"); // [cite: 116]
    Serial.print(" | PWM: "); Serial.print(base_pwm_power); // [cite: 116]
    Serial.print(" | Durée: "); Serial.println(duration_ms); // [cite: 116]
    setMotorPower(LEFT_MOTOR, direction, base_pwm_power); // [cite: 117]
    setMotorPower(RIGHT_MOTOR, direction, base_pwm_power); // [cite: 117]
    delay(duration_ms); // [cite: 117]
    stopMotors(); // [cite: 117]
}

void turnRobotPivot(bool clockwise, int speed_pwm) {
    if (clockwise) { // [cite: 19]
        setMotorPower(RIGHT_MOTOR, BACKWARD, speed_pwm); // [cite: 19]
        setMotorPower(LEFT_MOTOR, FORWARD, speed_pwm); // [cite: 20]
    } else { // [cite: 20]
        setMotorPower(RIGHT_MOTOR, FORWARD, speed_pwm); // [cite: 20]
        setMotorPower(LEFT_MOTOR, BACKWARD, speed_pwm); // [cite: 20]
    }
}

void turnRobotArc(int turn_direction, int base_pwm_power, unsigned long duration_ms) {
    Serial.print("Tourne (ARC) -> Dir: "); // [cite: 119]
    Serial.print(turn_direction == TURN_LEFT ? "GAUCHE" : "DROITE"); // [cite: 120]
    Serial.print(" | PWM Ext: "); Serial.print(base_pwm_power); // [cite: 120]
    Serial.print(" | Ratio Int: "); Serial.print(INNER_WHEEL_SPEED_RATIO_ARC_TURN); // [cite: 120]
    Serial.print(" | Durée: "); Serial.println(duration_ms); // [cite: 121]

    int pwm_outer = base_pwm_power; // [cite: 121]
    int pwm_inner = constrain((int)(base_pwm_power * INNER_WHEEL_SPEED_RATIO_ARC_TURN), 0, PWM_MAX); // [cite: 121]

    if (turn_direction == TURN_LEFT) { // [cite: 122]
        setMotorPower(RIGHT_MOTOR, FORWARD, pwm_outer); // [cite: 122]
        setMotorPower(LEFT_MOTOR, FORWARD, pwm_inner); // [cite: 123]
    } else if (turn_direction == TURN_RIGHT) { // [cite: 123]
        setMotorPower(LEFT_MOTOR, FORWARD, pwm_outer); // [cite: 123]
        setMotorPower(RIGHT_MOTOR, FORWARD, pwm_inner); // [cite: 124]
    }
    delay(duration_ms); // [cite: 124]
    stopMotors(); // [cite: 124]
}

void turnRobotTimed(bool clockwise, int speed_pwm, unsigned long duration_ms) {
    Serial.print("Tourne (Pivot/Temps): "); Serial.print(duration_ms); // [cite: 40]
    Serial.print("ms, direction: "); // [cite: 40]
    Serial.println(clockwise ? "Droite (CW)" : "Gauche (CCW)"); // [cite: 41]
    turnRobotPivot(clockwise, speed_pwm); // [cite: 41]
    delay(duration_ms); // [cite: 41]
    stopMotors(); // [cite: 42]
}

void moveForwardTimed(int speed_pwm, unsigned long duration_ms) {
    Serial.print("Avance (Temps) -> Duree: "); // [cite: 72]
    Serial.print(duration_ms); Serial.print("ms à vitesse "); Serial.println(speed_pwm); // [cite: 73]
    setMotorPower(LEFT_MOTOR, FORWARD, speed_pwm); // [cite: 76]
    setMotorPower(RIGHT_MOTOR, FORWARD, speed_pwm); // [cite: 76]
    delay(duration_ms); // [cite: 77]
    stopMotors(); // [cite: 78]
}

// --- Fonctions Magnétomètre ---
void calibrateMagnetometer() {
    Serial.println("--- DEBUT CALIBRATION ---"); // [cite: 21]
    SerialBT.println("Calibration..."); // [cite: 22]
    digitalWrite(LEDU1, HIGH); // [cite: 22]
    int16_t min_x = 32767, max_x = -32768, min_y = 32767, max_y = -32768, min_z = 32767, max_z = -32768; // [cite: 22]
    unsigned long start_time = millis(); // [cite: 23]
    turnRobotPivot(false, CALIBRATION_ROTATE_SPEED_PWM); // [cite: 23]
    while (millis() - start_time < CALIBRATION_DURATION_MS) {
        lis3mdl.read(); // [cite: 23]
        min_x = min(min_x, lis3mdl.x); max_x = max(max_x, lis3mdl.x); // [cite: 24]
        min_y = min(min_y, lis3mdl.y); max_y = max(max_y, lis3mdl.y); // [cite: 24]
        min_z = min(min_z, lis3mdl.z); max_z = max(max_z, lis3mdl.z); // [cite: 24]
        delay(20); // [cite: 25]
    }
    stopMotors(); // [cite: 25]
    mag_offset_x = (float)(min_x + max_x) / 2.0f; // [cite: 25]
    mag_offset_y = (float)(min_y + max_y) / 2.0f; // [cite: 26]
    mag_offset_z = (float)(min_z + max_z) / 2.0f; // [cite: 26]
    Serial.println("--- FIN CALIBRATION ---"); // [cite: 26]
    SerialBT.println("Calibration terminee."); // [cite: 27]
    digitalWrite(LEDU1, LOW); // [cite: 27]
}

float calculateTrueNorthHeading() {
    lis3mdl.read(); // [cite: 27]
    float corrected_x = (float)lis3mdl.x - mag_offset_x; // [cite: 28]
    float corrected_y = (float)lis3mdl.y - mag_offset_y; // [cite: 29]
    float heading_rad = atan2(corrected_x, corrected_y); // [cite: 30]
    float heading_deg = heading_rad * 180.0f / PI; // [cite: 30]
    heading_deg += 180.0f; // Correction pour pointer Nord [cite: 36]
    heading_deg += MAGNETIC_DECLINATION_DEG; // [cite: 37]
    while (heading_deg < 0.0f) heading_deg += 360.0f; // [cite: 38]
    while (heading_deg >= 360.0f) heading_deg -= 360.0f; // [cite: 39]
    return heading_deg; // [cite: 39]
}

void orientNorth(int speed_pwm) {
    Serial.println("--- DEBUT ORIENTATION NORD ---"); // [cite: 42]
    SerialBT.println("Orientation Nord..."); // [cite: 43]
    digitalWrite(LEDU1, HIGH);
    float target_angle = 0.0f;
    unsigned long timeout = millis() + 20000; // 20s max [cite: 43]
    float current_heading = calculateTrueNorthHeading(); // [cite: 44]
    Serial.print("Cap initial: "); Serial.println(current_heading); // [cite: 45]

    if ((current_heading >= (360.0f - HEADING_ACCURACY_DEG)) || (current_heading <= HEADING_ACCURACY_DEG)) { // [cite: 45]
        Serial.println(">>> Déjà orienté Nord ! <<<"); // [cite: 45]
        stopMotors(); digitalWrite(LEDU1, LOW); return; // [cite: 46]
    }

    bool turn_clockwise = (current_heading > HEADING_ACCURACY_DEG && current_heading < 180.0f) ? false : true; // [cite: 46, 47, 48]
    Serial.println(turn_clockwise ? "Décision: Tourner DROITE (CW) vers Nord." : "Décision: Tourner GAUCHE (CCW) vers Nord."); // [cite: 48, 49]
    turnRobotPivot(turn_clockwise, speed_pwm); // [cite: 49]

    while (millis() < timeout) {
        current_heading = calculateTrueNorthHeading(); // [cite: 50]
        // Serial.print("Cap: "); Serial.println(current_heading); // Optionnel
        SerialBT.print("Cap:"); SerialBT.println(current_heading); // [cite: 51]
        bool is_north = (current_heading >= (360.0f - HEADING_ACCURACY_DEG)) || (current_heading <= HEADING_ACCURACY_DEG); // [cite: 51]
        if (is_north) { Serial.println(">>> Cible Nord atteinte ! ARRET. <<<"); break; } // [cite: 52]
        delay(40); // [cite: 53]
    }
    stopMotors(); // [cite: 54]
    if (millis() >= timeout) { Serial.println(">>> TIMEOUT orientation! <<<"); } // [cite: 55]
    Serial.print("--- FIN. Cap final: "); Serial.println(calculateTrueNorthHeading()); // [cite: 55]
    digitalWrite(LEDU1, LOW); // [cite: 56]
}

// --- Fonctions Dessin ---
void drawTrianglePoint() {
    Serial.println("--- DESSIN POINTE TRIANGLE ---"); // [cite: 56]
    SerialBT.println("Dessin Pointe..."); // [cite: 57]
    digitalWrite(LEDU1, HIGH); // [cite: 57]
    unsigned long first_left_turn_duration; // [cite: 58]
    unsigned long recenter_left_turn_duration; // [cite: 58]

    for (int i = 0; i < NUM_TRIANGLE_STROKES; ++i) { // [cite: 59]
        unsigned long turn_duration_one_side = map(i, 0, NUM_TRIANGLE_STROKES - 1, MAX_TRIANGLE_TURN_DURATION_MS, MIN_TRIANGLE_TURN_DURATION_MS); // [cite: 60]
        turn_duration_one_side = max(turn_duration_one_side, (unsigned long)10); // [cite: 61]
        
        if (i == 0) { first_left_turn_duration = turn_duration_one_side / 2; } // [cite: 62]
        else { first_left_turn_duration = turn_duration_one_side; } // [cite: 63]
        recenter_left_turn_duration = first_left_turn_duration; // [cite: 64]

        Serial.print("  Stroke "); Serial.print(i + 1); // [cite: 65]
        Serial.print(", G1:"); Serial.print(first_left_turn_duration); // [cite: 65]
        Serial.print(", D:"); Serial.print(turn_duration_one_side); // [cite: 65]
        Serial.print(", G2:"); Serial.println(recenter_left_turn_duration); // [cite: 66]
        SerialBT.print("Stroke "); SerialBT.print(i + 1); SerialBT.print("/"); SerialBT.println(NUM_TRIANGLE_STROKES); // [cite: 66]

        turnRobotTimed(false, TRIANGLE_TURN_SPEED_PWM, first_left_turn_duration); delay(50); // [cite: 67]
        moveForwardTimed(TRIANGLE_STROKE_ADVANCE_SPEED_PWM, TRIANGLE_STROKE_SEGMENT_ADVANCE_DURATION_MS); delay(50); // [cite: 68]
        turnRobotTimed(true, TRIANGLE_TURN_SPEED_PWM, turn_duration_one_side); delay(50); // [cite: 68]
        moveForwardTimed(TRIANGLE_STROKE_ADVANCE_SPEED_PWM, TRIANGLE_STROKE_SEGMENT_ADVANCE_DURATION_MS); delay(50); // [cite: 69]
        turnRobotTimed(false, TRIANGLE_TURN_SPEED_PWM, recenter_left_turn_duration); delay(100); // [cite: 69]

        if (i < NUM_TRIANGLE_STROKES - 1) { // [cite: 70]
            moveForwardTimed(TRIANGLE_BASE_ADVANCE_SPEED_PWM, TRIANGLE_INTER_STROKE_ADVANCE_DURATION_MS); delay(100); // [cite: 70]
        }
    }
    digitalWrite(LEDU1, LOW); // [cite: 71]
    Serial.println("--- FIN DESSIN POINTE ---"); // [cite: 72]
}

// --- Séquences ---
void sequenceEscalier() {
    if (sequence_running) { Serial.println("Sequence deja en cours !"); return; }
    sequence_running = true;
    Serial.println("--- DEBUT SEQUENCE ESCALIER ---"); // [cite: 124]
    SerialBT.println("Debut Escalier..."); // [cite: 125]
    digitalWrite(LEDU1, HIGH);

    Serial.println("Etape 1: Avancer (20cm)"); // [cite: 125]
    moveRobotStraight(FORWARD, BASE_PWM_STRAIGHT_ESC, DURATION_FWD_20CM); delay(1000); // [cite: 125]
    Serial.println("Etape 2: Tourner Gauche (90deg)"); // [cite: 126]
    turnRobotArc(TURN_LEFT, BASE_PWM_TURN_ESC, DURATION_TURN_90DEG_ESC_1); delay(1000); // [cite: 126]
    Serial.println("Etape 3: Avancer (10cm)"); // [cite: 127]
    moveRobotStraight(FORWARD, BASE_PWM_STRAIGHT_ESC, DURATION_FWD_10CM); delay(1000); // [cite: 127]
    Serial.println("Etape 4: Tourner Droite (90deg)"); // [cite: 127]
    turnRobotArc(TURN_RIGHT, BASE_PWM_TURN_ESC, DURATION_TURN_90DEG_ESC_2); delay(1000); // [cite: 128]
    Serial.println("Etape 5: Avancer (40cm)"); // [cite: 128]
    moveRobotStraight(FORWARD, BASE_PWM_STRAIGHT_ESC, DURATION_FWD_40CM); // [cite: 128]

    Serial.println("--- FIN SEQUENCE ESCALIER ---"); // [cite: 129]
    SerialBT.println("Escalier terminé."); // [cite: 129]
    digitalWrite(LEDU1, LOW);
    sequence_running = false;
}

void sequenceFleche() {
    if (sequence_running) { Serial.println("Sequence deja en cours !"); return; } // [cite: 80]
    sequence_running = true;
    Serial.println(">>> DEBUT SEQUENCE FLECHE <<<"); // [cite: 81]
    SerialBT.println(">>> Demarrage Fleche <<<"); // [cite: 81]
    digitalWrite(LEDU1, HIGH); // [cite: 82]

    calibrateMagnetometer(); delay(1000); // [cite: 83]
    orientNorth(ORIENTATION_ROTATE_SPEED_PWM); delay(1000); // [cite: 83]
    moveForwardTimed(FORWARD_MOVE_SPEED_PWM, FORWARD_MOVE_DURATION_MS); delay(1000); // [cite: 84]
    drawTrianglePoint(); delay(1000); // [cite: 85]

    digitalWrite(LEDU1, LOW); // [cite: 86]
    Serial.println(">>> FIN SEQUENCE FLECHE <<<"); // [cite: 87]
    SerialBT.println(">>> FIN - ARRET TOTAL <<<"); // [cite: 87]
    sequence_running = false; // [cite: 87]
}

// --- Communication ---
void handleBluetoothCommands() {
    if (SerialBT.available()) {
        char command = SerialBT.read(); // [cite: 88]
        if (!sequence_running) {
            if (command == '1') {
                sequence_escalier_requested = true; // [cite: 130]
                Serial.println(">>> Commande '1' (Escalier) recue <<<");
            } else if (command == '3' || command == 'S' || command == 's') {
                sequence_fleche_requested = true; // [cite: 89]
                Serial.println(">>> Commande '3' (Fleche) recue <<<"); // [cite: 90]
            }
        }
        while(SerialBT.available()) { SerialBT.read(); } // Vide le buffer [cite: 90]
    }
}

// --- Setup et Loop ---
void setup() {
    Serial.begin(115200); // [cite: 91]
    delay(500);
    Serial.println("\n\n--- INITIALISATION DRAWBOT FUSION ---"); // [cite: 92]
    pinMode(LEDU1, OUTPUT); digitalWrite(LEDU1, LOW); // [cite: 92]
    Wire.begin(SDA_PIN, SCL_PIN); // [cite: 92]
    setupLIS3MDL(); // [cite: 92]
    setupMotors(); // [cite: 92]
    setupBluetooth(); // [cite: 92]
    Serial.println("--- Pret. Attente Commandes BT ('1' ou '3'). ---"); // [cite: 93]
    SerialBT.println("Attente '1' ou '3'...");
}

void loop() {
    handleBluetoothCommands(); // [cite: 93]
    if (sequence_escalier_requested) {
        sequence_escalier_requested = false;
        sequenceEscalier();
        Serial.println(">>> Attente nouvelle commande. <<<"); // [cite: 95]
        SerialBT.println("Attente '1' ou '3'...");
    }
    if (sequence_fleche_requested) {
        sequence_fleche_requested = false;
        sequenceFleche(); // [cite: 94]
        Serial.println(">>> Attente nouvelle commande. <<<"); // [cite: 95]
        SerialBT.println("Attente '1' ou '3'...");
    }
    delay(200); // [cite: 95]
}