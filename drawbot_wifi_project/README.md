# Drawbot WiFi ESP32

Projet PlatformIO / Arduino ESP32 pour controler un robot a deux moteurs depuis une interface web.

## Utilisation

1. Ouvrir le projet avec PlatformIO.
2. Compiler puis televerser sur une carte ESP32 NodeMCU.
3. Au demarrage, l'ESP32 cree le reseau WiFi `Drawbot`.
4. Se connecter au reseau avec le mot de passe `drawbot123`.
5. Ouvrir `http://192.168.4.1`.

Le bouton BOOT (GPIO0) permet ensuite de couper ou relancer le WiFi.

## Interface web

L'interface contient :

- un D-pad ;
- un joystick ;
- des commandes manuelles `FWD/BCK/TL/TR,pwm,ms` ;
- les profils Lent, Normal et Rapide ;
- les sequences S1 escalier, S2 cercle simple, S3 fleche Nord.

## Commandes utiles

```bash
pio run
pio run --target upload
pio device monitor -b 115200
```
