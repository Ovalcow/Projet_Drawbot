# Drawbot WiFi ESP32

Projet ESP32 pour contrôler un robot à deux moteurs depuis une interface web.

## Utilisation

1. Ouvre le projet avec PlatformIO.
2. Téléverse sur une carte ESP32.
3. Appuie sur le bouton BOOT pour activer le WiFi.
4. Connecte-toi au réseau WiFi : `Drawbot`.
5. Mot de passe : `drawbot123`.
6. Ouvre : `http://192.168.4.1`.

## Interface web

L'interface contient un mode libre au joystick, un D-pad, les modes Escalier/Carré/Zigzag, les profils de vitesse et une commande manuelle.

## Correction incluse

`WIFI_AP_IP` est défini dans `include/config.h`, ce qui corrige l'erreur de compilation dans `src/main.cpp`.
