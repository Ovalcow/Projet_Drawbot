#ifndef WIFI_SERVER_H
#define WIFI_SERVER_H

#include <Arduino.h>

// Initialise le bouton, la LED, le point d'acces WiFi et le serveur HTTP.
void setupWifi();

// A appeler dans loop() : gere le bouton BOOT et les requetes HTTP.
void loopWifi();

// Active le point d'acces WiFi et le serveur HTTP.
void enableWifi();

// Coupe le WiFi et le serveur HTTP.
void disableWifi();

// Traite une commande texte recue par HTTP.
void handleCommand(const String& cmd, String& response);

#endif // WIFI_SERVER_H
