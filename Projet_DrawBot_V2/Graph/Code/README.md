# Projet Drawbot - Systèmes Bouclés ECE

Bienvenue dans le dépôt du code source pour le projet Drawbot. Ce projet s'inscrit dans le cadre de l'enseignement "Systèmes Bouclés" de l'ECE Paris [cite: 1] et vise à développer un robot dessinateur, le Drawbot, basé sur la plateforme Gyrobot[cite: 3].

## Description Générale

Le Drawbot est un robot mobile conçu pour dessiner des formes et des séquences complexes sur une surface plane. Il est contrôlé à distance par une application s'exécutant sur un ordinateur, qui communique via Wi-Fi avec une carte de développement NodeMCU ESP32 embarquée sur le robot[cite: 13]. Le robot utilise divers capteurs (encodeurs moteurs, centrale inertielle, magnétomètre) pour un contrôle précis de ses mouvements et de son orientation[cite: 17, 18], en s'appuyant sur des boucles de rétroaction et des correcteurs PID[cite: 2].

**Objectifs pédagogiques principaux (selon CDC.pdf) :**
* Application des acquis en commande d'actionneur, acquisition de capteurs, et correction PID numérique (asservissement)[cite: 2].
* Développement de compétences en méthodologie de projet (cycle en V), documentation technique, et respect des deadlines[cite: 2].
* Mise en œuvre d'une communication sans fil entre un ordinateur et un système embarqué.
* Réalisation de séquences de dessin spécifiques :
    * Séquence 1 : L'escalier (ligne, rotation, ligne, rotation, ligne)[cite: 14, 77].
    * Séquence 2 : Le cercle (rayon paramétrable)[cite: 15, 90].
    * Séquence 3 : La rose des vents ou une flèche dirigée vers le Nord terrestre[cite: 16, 103].

## Plateforme Matérielle & Technologies

* **Robot :** Plateforme Gyrobot ECE[cite: 3].
* **Microcontrôleur embarqué :** Carte de développement NodeMCU ESP32[cite: 3, 8, 9].
    * Microcontrôleur : ESP32[cite: 9].
    * Connectivité sans fil : Wi-Fi (2,4 GHz), Bluetooth[cite: 9].
    * Niveau logique des GPIO : 3,3 V[cite: 9].
* **Firmware (embarqué) :** Développé en C++ avec le framework Arduino.
* **Application de contrôle (PC) :** Développée en Python.
* **Capteurs principaux (embarqués) :**
    * Deux encodeurs de moteurs (un pour chaque roue)[cite: 17].
    * Une centrale inertielle (IMU LSM6DS3)[cite: 3, 6, 18].
    * Un magnétomètre (LIS3MDL)[cite: 3, 6, 18].
* **Actionneurs :** Motoréducteurs N20 à 100 RPM avec drivers DRV8837DSGR[cite: 3, 6].

## Document de Référence Principal

Les spécifications complètes, les objectifs détaillés, les critères de notation, et les instructions d'assemblage se trouvent dans le document `CDC.pdf` ("SYSTÈMES BOUCLÉS - Kick-off meeting DRAWBOT") fourni par l'ECE[cite: 1]. Ce document est la source de vérité pour les exigences du projet.

## Structure de ce Dépôt de Code

* **`/Firmware_ESP32`**: Contient tout le code source C++/Arduino destiné à être compilé et téléversé sur la carte NodeMCU ESP32 du robot.
* **`/Controle_PC`**: Contient le code source Python de l'application qui s'exécute sur l'ordinateur pour envoyer des commandes au Drawbot.
* **`/Documentation`** (suggestion, à créer par vos soins) : Pourrait contenir les diagrammes fonctionnels[cite: 47], l'architecture matérielle[cite: 50], les schématiques KiCAD/Altium du Gyrobot[cite: 52], et autres documents de conception relatifs au projet.

## Configuration Générale Requise

* **Pour le Firmware_ESP32 :**
    * VS Code avec l'extension PlatformIO IDE.
    * Drivers USB pour la carte NodeMCU ESP32 (généralement CH340/CH341 ou CP210x selon la version de la carte).
* **Pour le Controle_PC :**
    * Python 3.x.
    * Gestionnaire de paquets `pip`.
    * Il est fortement recommandé d'utiliser un environnement virtuel Python.

Ce README fournit une vue d'ensemble. Pour des détails spécifiques au firmware ou à l'application PC, veuillez consulter les README respectifs dans les sous-dossiers.