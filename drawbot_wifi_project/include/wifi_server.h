#include <Arduino.h>
#ifndef WIFI_SERVER_H
#define WIFI_SERVER_H

// Initialise GPIO bouton/LED, ne démarre pas le WiFi
void setupWifi();

// À appeler dans loop() : gère le bouton BOOT + les requêtes HTTP
void loopWifi();

// Active le point d'accès WiFi et le serveur HTTP
void enableWifi();

// Coupe le WiFi et le serveur HTTP
void disableWifi();

// Traite une commande texte (identique aux anciennes commandes BT)
void handleCommand(const String& cmd, String& response);

#endif // WIFI_SERVER_H