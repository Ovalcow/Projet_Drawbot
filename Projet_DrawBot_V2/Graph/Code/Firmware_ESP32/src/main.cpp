#include <Arduino.h>
#include <BluetoothSerial.h>
#include "config.h" 
#include "ControleurPID.h" // Notre classe PID

// --- Bluetooth ---
BluetoothSerial SerialBT_INSTANCE;
String btCommandeRecue = "";
bool btClientConnecte = false;

// --- Encodeurs ---
volatile long compteurEncodeurG = 0;
volatile long compteurEncodeurD = 0;

// --- Mode Asservissement (ASSER) ---
bool asserModeActif = false;
ControleurPID pidGauche_asser(ASSER_DEFAULT_KP, ASSER_DEFAULT_KI, ASSER_DEFAULT_KD, ASSER_PID_OUTPUT_LIMIT);
ControleurPID pidDroit_asser(ASSER_DEFAULT_KP, ASSER_DEFAULT_KI, ASSER_DEFAULT_KD, ASSER_PID_OUTPUT_LIMIT);
long consigneEncG_asser = 0; // Cible en ticks pour l'encodeur gauche
long consigneEncD_asser = 0; // Cible en ticks pour l'encodeur droit

// --- Teleplot ---
unsigned long dernierTempsTeleplot = 0;
const long intervalleTeleplot = 100; // ms (10 Hz)

// --- Prototypes ---
void setupBluetooth();
void lireEtTraiterCommandeBluetooth();
void envoyerMsgBT(const String& message); 
void callbackBT(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
void setupEncodeurs();
void razEncodeurs(); 
long getEncG(); 
long getEncD(); 
void IRAM_ATTR isrEncG_A(); void IRAM_ATTR isrEncG_B(); void IRAM_ATTR isrEncD_A(); void IRAM_ATTR isrEncD_B();
void setupMoteurs(); 
void cmdMoteur(int pinEN, int pinIN1, int pinIN2, int puissance, bool estDroit);
void stopMoteurs(); 
void appliquerPuissanceMoteurs(int puissanceG, int puissanceD); 
bool extraireValeurFlottanteCmd(const String& cmdStr, const String& motCle, float& valeur);
void executerModeAsservissement();
void envoyerDonneesTeleplotAsser();


// --- Implémentation Bluetooth ---
void callbackBT(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) { /* ... identique ... */ 
    if (event == ESP_SPP_SRV_OPEN_EVT) { Serial.println("Client BT Connecté"); btClientConnecte = true; } 
    else if (event == ESP_SPP_CLOSE_EVT) { Serial.println("Client BT Déconnecté"); btClientConnecte = false; }
}
void setupBluetooth() { /* ... identique ... */ 
    if (!SerialBT_INSTANCE.begin(BLUETOOTH_DEVICE_NAME)) { Serial.println("ERREUR Init BT!"); } 
    else { Serial.print("BT OK. Nom: "); Serial.println(BLUETOOTH_DEVICE_NAME); }
    SerialBT_INSTANCE.register_callback(callbackBT);
}
void envoyerMsgBT(const String& message) { /* ... identique ... */ 
    if (btClientConnecte) { SerialBT_INSTANCE.println(message); }
    Serial.print("BT Msg: ["); Serial.print(message); Serial.println("]");
}

// --- Implémentation Encodeurs ---
void IRAM_ATTR isrEncG_A() { /* ... identique ... */ int valB = digitalRead(ENCODEUR_G_CHB_PIN); if (digitalRead(ENCODEUR_G_CHA_PIN) == valB) { compteurEncodeurG--; } else { compteurEncodeurG++; }}
void IRAM_ATTR isrEncG_B() { /* ... identique ... */ int valA = digitalRead(ENCODEUR_G_CHA_PIN); if (digitalRead(ENCODEUR_G_CHB_PIN) != valA) { compteurEncodeurG--; } else { compteurEncodeurG++; }}
void IRAM_ATTR isrEncD_A() { /* ... identique ... */ int valB = digitalRead(ENCODEUR_D_CHA_PIN); if (digitalRead(ENCODEUR_D_CHA_PIN) != valB) { compteurEncodeurD--; } else { compteurEncodeurD++; }}
void IRAM_ATTR isrEncD_B() { /* ... identique ... */ int valA = digitalRead(ENCODEUR_D_CHA_PIN); if (digitalRead(ENCODEUR_D_CHB_PIN) == valA) { compteurEncodeurD--; } else { compteurEncodeurD++; }}
void setupEncodeurs() { /* ... identique ... */
    pinMode(ENCODEUR_G_CHA_PIN, INPUT_PULLUP); pinMode(ENCODEUR_G_CHB_PIN, INPUT_PULLUP);
    pinMode(ENCODEUR_D_CHA_PIN, INPUT_PULLUP); pinMode(ENCODEUR_D_CHB_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ENCODEUR_G_CHA_PIN), isrEncG_A, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODEUR_G_CHB_PIN), isrEncG_B, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODEUR_D_CHA_PIN), isrEncD_A, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODEUR_D_CHB_PIN), isrEncD_B, CHANGE);
    compteurEncodeurG = 0; compteurEncodeurD = 0; Serial.println("Encodeurs initialisés et RAZ.");
}
void razEncodeurs() { /* ... identique ... */ noInterrupts(); compteurEncodeurG = 0; compteurEncodeurD = 0; interrupts(); Serial.println("Compteurs encodeurs RAZ.");}
long getEncG() { long val; noInterrupts(); val = compteurEncodeurG; interrupts(); return val; }
long getEncD() { long val; noInterrupts(); val = compteurEncodeurD; interrupts(); return val; }

// --- Implémentation Moteurs ---
void setupMoteurs() { /* ... identique ... */
    pinMode(MOTEUR_D_EN_PIN, OUTPUT); pinMode(MOTEUR_D_IN1_PIN, OUTPUT); pinMode(MOTEUR_D_IN2_PIN, OUTPUT);
    pinMode(MOTEUR_G_EN_PIN, OUTPUT); pinMode(MOTEUR_G_IN1_PIN, OUTPUT); pinMode(MOTEUR_G_IN2_PIN, OUTPUT);
    digitalWrite(MOTEUR_D_EN_PIN, LOW); digitalWrite(MOTEUR_G_EN_PIN, LOW); 
    Serial.println("Moteurs initialisés (désactivés).");
}
void cmdMoteur(int pinEN, int pinIN1, int pinIN2, int puissance, bool estDroit) { /* ... identique ... */
    if (puissance == 0) { digitalWrite(pinEN, HIGH); digitalWrite(pinIN1, LOW); digitalWrite(pinIN2, LOW); return; } // Freinage actif
    digitalWrite(pinEN, HIGH); int pAbs = abs(puissance); if (pAbs > 255) pAbs = 255;
    bool avant = (puissance > 0); // Convention: puissance positive = AVANT pour le PID
    if (estDroit) { // Logique moteur droit pour AVANT (commande PID positive)
        if (avant) { digitalWrite(pinIN1, LOW); analogWrite(pinIN2, pAbs); } 
        else { analogWrite(pinIN1, pAbs); digitalWrite(pinIN2, LOW); } // ARRIERE
    } else { // Logique moteur gauche pour AVANT (commande PID positive)
        if (avant) { analogWrite(pinIN1, pAbs); digitalWrite(pinIN2, LOW); } 
        else { digitalWrite(pinIN1, LOW); analogWrite(pinIN2, pAbs); } // ARRIERE
    }}
void appliquerPuissanceMoteurs(int puissanceG, int puissanceD) { /* ... identique ... */
    cmdMoteur(MOTEUR_G_EN_PIN, MOTEUR_G_IN1_PIN, MOTEUR_G_IN2_PIN, puissanceG, false);
    cmdMoteur(MOTEUR_D_EN_PIN, MOTEUR_D_IN1_PIN, MOTEUR_D_IN2_PIN, puissanceD, true);
}
void stopMoteurs() { /* ... identique ... */
    Serial.println("Moteurs: STOP.");
    cmdMoteur(MOTEUR_G_EN_PIN, MOTEUR_G_IN1_PIN, MOTEUR_G_IN2_PIN, 0, false); 
    cmdMoteur(MOTEUR_D_EN_PIN, MOTEUR_D_IN1_PIN, MOTEUR_D_IN2_PIN, 0, true);  
    delay(50); 
    digitalWrite(MOTEUR_G_EN_PIN, LOW); 
    digitalWrite(MOTEUR_D_EN_PIN, LOW);
}

// --- Utilitaires ---
bool extraireValeurFlottanteCmd(const String& cmdStr, const String& motCle, float& valeur) {
    if (cmdStr.startsWith(motCle)) {
        String valHex = cmdStr.substring(motCle.length());
        valHex.trim();
        if (valHex.length() > 0) {
            valeur = valHex.toFloat();
            return true;
        }
    }
    return false;
}

// --- Logique ASSER et Teleplot ---
void envoyerDonneesTeleplotAsser() {
    if (!asserModeActif && !btClientConnecte) return; // N'envoyer que si pertinent

    long currentEncG = getEncG();
    long currentEncD = getEncD();

    Serial.print(">consigneG:"); Serial.println(pidGauche_asser.getConsigne());
    Serial.print(">posG:"); Serial.println(currentEncG);
    // Serial.print(">erreurG:"); Serial.println(pidGauche_asser.getConsigne() - currentEncG); // Peut être calculé dans Teleplot
    // Pour afficher la sortie PID, il faudrait la stocker après l'appel à .calculer()
    
    Serial.print(">consigneD:"); Serial.println(pidDroit_asser.getConsigne());
    Serial.print(">posD:"); Serial.println(currentEncD);
    // Serial.print(">erreurD:"); Serial.println(pidDroit_asser.getConsigne() - currentEncD);
    
    Serial.print(">kp:"); Serial.println(pidGauche_asser.getKp());
    Serial.print(">ki:"); Serial.println(pidGauche_asser.getKi());
    Serial.print(">kd:"); Serial.println(pidGauche_asser.getKd());
}

void executerModeAsservissement() {
    if (!asserModeActif) {
        // Si le mode vient d'être désactivé, s'assurer que les moteurs sont relâchés
        // stopMoteurs(); // Normalement déjà fait par la commande OFF
        return;
    }

    long valG = getEncG();
    long valD = getEncD();

    // Les PIDs travaillent pour maintenir la consigne (qui est 0 relatif au point de départ)
    // Une sortie PID positive signifie que le moteur doit tourner en "avant" pour corriger une pos < consigne
    // Une sortie PID négative signifie que le moteur doit tourner en "arrière" pour corriger une pos > consigne
    double cmdPidG = pidGauche_asser.calculer(valG);
    double cmdPidD = pidDroit_asser.calculer(valD);

    appliquerPuissanceMoteurs(static_cast<int>(cmdPidG), static_cast<int>(cmdPidD));
}

// --- Setup & Loop ---
void setup() {
    Serial.begin(115200);
    Serial.println("=== DEMARRAGE DRAWBOT - Mode ASSER PID Tune ===");
    pinMode(LEDU1_PIN, OUTPUT); digitalWrite(LEDU1_PIN, LOW); 
    
    setupEncodeurs(); // Initialise et RAZ les compteurs
    setupMoteurs(); 
    setupBluetooth();    

    // Initialiser les PIDs avec les valeurs par défaut de config.h
    pidGauche_asser.setKp(ASSER_DEFAULT_KP);
    pidGauche_asser.setKi(ASSER_DEFAULT_KI);
    pidGauche_asser.setKd(ASSER_DEFAULT_KD);
    pidDroit_asser.setKp(ASSER_DEFAULT_KP);
    pidDroit_asser.setKi(ASSER_DEFAULT_KI);
    pidDroit_asser.setKd(ASSER_DEFAULT_KD);
    
    // La consigne sera mise à jour lors de l'activation du mode ASSER
    pidGauche_asser.setConsigne(0); // Consigne initiale
    pidDroit_asser.setConsigne(0);  // Consigne initiale

    Serial.println("Systeme PRET. Cmds: ASSER, OFF, SETP/I/D <val>, RESETENC");
}

bool btAnciennementConnecte = false; 

void loop() {
    if (btClientConnecte && !btAnciennementConnecte) {
        digitalWrite(LEDU1_PIN, HIGH); 
        Serial.println("Client BT Connecté.");
        envoyerMsgBT("Connecte! Cmds: ASSER, OFF, SETP/I/D <val>, RESETENC");
    } else if (!btClientConnecte && btAnciennementConnecte) {
        digitalWrite(LEDU1_PIN, LOW); 
        Serial.println("Client BT Déconnecté.");
        if (asserModeActif) { // Si déconnecté pendant ASSER
            asserModeActif = false;
            stopMoteurs();
            envoyerMsgBT("BT Deco: Mode ASSER Stoppe."); // Ne sera pas reçu mais bon pour log Serial
        }
    }
    btAnciennementConnecte = btClientConnecte;

    if (btClientConnecte) {
        // Lire et traiter une commande Bluetooth
        if (SerialBT_INSTANCE.available()) {
            btCommandeRecue = SerialBT_INSTANCE.readStringUntil('\n');
            btCommandeRecue.trim();
            String cmdUpper = btCommandeRecue; // Garder une copie pour comparaison insensible à la casse
            cmdUpper.toUpperCase();
            Serial.print("BT Cmd Recue: ["); Serial.print(btCommandeRecue); Serial.println("]");

            float valeurGain;
            if (cmdUpper == "ASSER") {
                asserModeActif = true;
                razEncodeurs(); // La position actuelle devient la consigne 0
                consigneEncG_asser = 0; 
                consigneEncD_asser = 0;
                pidGauche_asser.setConsigne(consigneEncG_asser);
                pidDroit_asser.setConsigne(consigneEncD_asser);
                pidGauche_asser.reset(); // Important pour l'intégral et le dérivé
                pidDroit_asser.reset();
                digitalWrite(LEDU1_PIN, HIGH); // Indiquer mode actif
                envoyerMsgBT("ASSER: Mode Maintien Position ACTIVE.");
                Serial.println("Mode ASSER activé. Cibles encodeurs = 0.");
            } else if (cmdUpper == "OFF") {
                asserModeActif = false;
                stopMoteurs();
                digitalWrite(LEDU1_PIN, LOW); // Si connecté, LED reste allumée, sinon elle s'éteint déjà
                envoyerMsgBT("ASSER: Mode Maintien Position DESACTIVE.");
                Serial.println("Mode ASSER désactivé.");
            } else if (cmdUpper.startsWith("SETP ")) {
                if(extraireValeurFlottanteCmd(btCommandeRecue, "SETP ", valeurGain)){
                    pidGauche_asser.setKp(valeurGain); pidDroit_asser.setKp(valeurGain);
                    envoyerMsgBT("ASSER Kp: " + String(valeurGain, 4));
                } else { envoyerMsgBT("Erreur: SETP <valeur_numerique>");}
            } else if (cmdUpper.startsWith("SETI ")) {
                if(extraireValeurFlottanteCmd(btCommandeRecue, "SETI ", valeurGain)){
                    pidGauche_asser.setKi(valeurGain); pidDroit_asser.setKi(valeurGain);
                    envoyerMsgBT("ASSER Ki: " + String(valeurGain, 4));
                } else { envoyerMsgBT("Erreur: SETI <valeur_numerique>");}
            } else if (cmdUpper.startsWith("SETD ")) {
                if(extraireValeurFlottanteCmd(btCommandeRecue, "SETD ", valeurGain)){
                    pidGauche_asser.setKd(valeurGain); pidDroit_asser.setKd(valeurGain);
                    envoyerMsgBT("ASSER Kd: " + String(valeurGain, 4));
                } else { envoyerMsgBT("Erreur: SETD <valeur_numerique>");}
            } else if (cmdUpper == "RESETENC") {
                razEncodeurs();
                // Si en mode ASSER, la consigne redevient la nouvelle position 0
                if (asserModeActif) {
                    consigneEncG_asser = 0; consigneEncD_asser = 0;
                    pidGauche_asser.setConsigne(consigneEncG_asser);
                    pidDroit_asser.setConsigne(consigneEncD_asser);
                    pidGauche_asser.reset(); pidDroit_asser.reset();
                }
                envoyerMsgBT("Encodeurs RAZ.");
            } else {
                envoyerMsgBT("Cmd inconnue: " + btCommandeRecue);
            }
            btCommandeRecue = ""; // Vider la commande après traitement
        }
    }

    if (asserModeActif) {
        executerModeAsservissement();
    }

    // Envoi des données Teleplot si le mode asser est actif ET un client BT est connecté
    // (pour ne pas spammer le port série si personne n'écoute ou si pas en mode pertinent)
    if (asserModeActif && btClientConnecte && (millis() - dernierTempsTeleplot >= intervalleTeleplot) ) {
        envoyerDonneesTeleplotAsser();
        dernierTempsTeleplot = millis();
    }
    
    delay(10); // Boucle principale à ~100Hz
}