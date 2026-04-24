# Firmware ESP32 pour le Projet Drawbot

Ce dossier contient le code source C++/Arduino pour le microcontrôleur NodeMCU ESP32 embarqué sur le robot Drawbot. Ce firmware est responsable du contrôle de bas niveau du robot, de la lecture des capteurs, de l'implémentation des boucles d'asservissement (PID)[cite: 2, 65], et de la communication avec l'application de contrôle sur PC.

## Matériel Cible et Brochage

* **Carte de développement :** NodeMCU ESP32[cite: 3, 8].
    * Microcontrôleur : ESP32 (240 MHz, 512 kB SRAM, 4 MB Flash, Wi-Fi, Bluetooth)[cite: 9].
    * Niveau logique GPIO : **3,3 V**[cite: 9].
* **Moteurs :** Deux motoréducteurs N20 à 100 RPM avec encodeurs à effet Hall[cite: 3].
* **Drivers Moteurs :** Deux DRV8837DSGR[cite: 3, 6].
* **Capteurs :**
    * **Centrale Inertielle (IMU) :** LSM6DS3[cite: 3, 6].
        * Communication : I2C.
        * Adresse I2C : `0x6B`[cite: 11].
    * **Magnétomètre (MAG) :** LIS3MDL[cite: 3, 6].
        * Communication : I2C.
        * Adresse I2C : `0x1E`[cite: 11].
    * **Encodeurs Moteurs :** Intégrés aux moteurs, fournissent des signaux pour mesurer la rotation des roues.

* **Brochage Principal (selon CDC Slide 6 [cite: 11]) :**
    * LEDs Utilisateur : `LEDU1 (GPIO25)`, `LEDU2 (GPIO26)`.
    * Enable Moteurs : **`EN_D (GPIO23)`** (Droit), **`EN_G (GPIO4)`** (Gauche).
    * Commande PWM Moteur Droit : `IN_1_D (GPIO19)`, `IN_2_D (GPIO18)`.
    * Commande PWM Moteur Gauche : `IN_1_G (GPIO17)`, `IN_2_G (GPIO16)`.
    * Encodeur Gauche : `ENC_G_CH_A (GPIO32)`, `ENC_G_CH_B (GPIO33)`.
    * Encodeur Droit : `ENC_D_CH_A (GPIO27)`, `ENC_D_CH_B (GPIO14)`.
    * Bus I2C (pour IMU, MAG) : **`SDA (GPIO21)`**, **`SCL (GPIO22)`**.

## Environnement de Développement

* **IDE :** Visual Studio Code.
* **Outil de build / Gestionnaire de projet :** PlatformIO.
* **Configuration PlatformIO (`platformio.ini`) :**
    ```ini
    [env:nodemcu-32s]
    platform = espressif32
    board = nodemcu-32s
    framework = arduino
    monitor_speed = 115200
    ; lib_deps =
    ;   adafruit/Adafruit LSM6DS @ x.y.z  ; Pour l'IMU (vérifier la version compatible)
    ;   adafruit/Adafruit LIS3MDL @ x.y.z ; Pour le magnétomètre (vérifier la version compatible)
    ;   # Autres bibliothèques (ex: pour un serveur Web asynchrone si besoin)
    ```
    *(Décommentez et ajoutez les `lib_deps` nécessaires. Utilisez le gestionnaire de bibliothèques de PlatformIO pour trouver les bonnes versions.)*

## Tâches et Fonctionnalités Clés du Firmware

1.  **Initialisation Matérielle :** Configuration des GPIOs, du bus I2C, de la communication série.
2.  **Contrôle des Moteurs :**
    * Génération de signaux PWM (via `analogWrite` ou la librairie LEDC de l'ESP32) pour le contrôle de vitesse sur les pins `IN1_x` / `IN2_x`.
    * Gestion de la direction via la logique des pins `IN1_x` et `IN2_x` pour les drivers DRV8837DSGR.
    * Activation/Désactivation des drivers via les pins `EN_D` et `EN_G`.
3.  **Lecture des Encodeurs :**
    * Utilisation d'interruptions externes sur les pins `ENC_x_CH_A` et `ENC_x_CH_B` pour compter les impulsions avec précision et déterminer le sens de rotation (décodage en quadrature).
    * Conversion des comptes d'encodeur en distance parcourue (nécessite le calibrage des actionneurs [cite: 68]).
4.  **Acquisition des Données Capteurs (IMU/MAG) :**
    * Communication via I2C en utilisant la bibliothèque `Wire.h` et des bibliothèques spécifiques aux capteurs (LSM6DS3, LIS3MDL).
    * Fusion de capteurs (optionnel, pour une meilleure estimation de l'orientation).
    * Calibrage de l'IMU et du magnétomètre (comparaison avec une boussole de référence)[cite: 69].
5.  **Implémentation des Correcteurs PID :**
    * Au moins un asservissement en position des roues est requis[cite: 59].
    * Des PIDs seront probablement nécessaires pour le contrôle de la distance, des angles de rotation, et potentiellement pour le suivi de cap (pour la Séquence 3).
    * Le réglage des paramètres $K_P, K_I, K_D$ est une étape cruciale[cite: 65, 71]. Documenter la méthodologie de réglage.
6.  **Communication Wi-Fi :**
    * Connexion de l'ESP32 à un réseau Wi-Fi.
    * Mise en place d'un serveur (ex: HTTP via `WebServer.h` ou `ESPAsyncWebServer`) pour écouter les commandes provenant de l'application PC.
7.  **Interprétation des Commandes :**
    * Analyse (parsing) des commandes reçues via Wi-Fi.
    * Exécution des logiques de mouvement correspondantes pour réaliser les séquences de dessin.
8.  **Gestion de l'Alimentation :**
    * Le robot est alimenté par 4 piles LR6 AA pour les phases de roulage[cite: 20, 45].
    * **Éteindre le robot (interrupteur SW1) en dehors des phases de test et lors des téléversements pour économiser les piles**[cite: 46].
    * Attention au sens de branchement du condensateur C18[cite: 27, 41, 42, 43, 44].

## Considérations Importantes (issues du CDC.pdf)

* **Fréquence d'échantillonnage :** À déterminer et à justifier par rapport au temps de réaction des actionneurs[cite: 63, 64].
* **Gestion des erreurs et robustesse :** Que se passe-t-il si une commande est mal formée ? Si la connexion Wi-Fi est perdue ?
* **Tests unitaires :** Essentiels pour valider chaque module (communication, capteurs, actionneurs)[cite: 67].
* **Non-utilisation de tous les capteurs :** Si certains capteurs ne sont pas utilisés, il faudra expliquer pourquoi[cite: 60].

## Compilation et Téléversement

1.  S'assurer que `platformio.ini` est correctement configuré (board, framework, monitor_speed, lib_deps).
2.  Utiliser les commandes PlatformIO dans VS Code :
    * `PlatformIO: Build` (pour compiler).
    * `PlatformIO: Upload` (pour téléverser sur le NodeMCU ESP32).
    * `PlatformIO: Clean` (pour nettoyer les fichiers de build).
    * `PlatformIO: Serial Monitor` (pour visualiser les `Serial.print` à la vitesse `monitor_speed`).

Ce README sert de point de départ. Il devra être mis à jour au fur et à mesure que le firmware évolue.