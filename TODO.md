# TODO — Drawbot WiFi (Plan 4 phases)

## Phase 1 — Corriger ce qui empêche les séquences de fonctionner (~2h)
- [x] 1.1 Unifier la logique du moteur droit : corriger sens FORWARD/BACKWARD dans `drawbot_wifi_project/src/motors.cpp` (IN1/IN2 côté droit).

- [ ] 1.1b (si applicable) supprimer les doublons `main.txt` / config dupliqués hors de `drawbot_wifi_project`.
- [x] 1.2 Corriger `dur_10cm` et `dur_90deg` dans `drawbot_wifi_project/include/config.h` (valeurs non nulles, calibrage empirique).

- [ ] 1.3 (plus tard) Fusion porter code : pas encore commencé dans Phase 1.
- [x] 1.4 Remplacer les virages de S1 par un pivot sur place : implémenter `turnRobotPivot()` puis remplacer `turnRobot()` dans `sequenceEscalier()`.


## Phase 2 — Implémenter ce qui manque complètement (~6h)
- [ ] 2.1 Encoders + fermeture de boucle : `moveDistance_cm()` et `rotateDeg()` + remplacement des `delay`.
- [ ] 2.2 Porter ControleurPID + 2 instances + deadzone.
- [ ] 2.3 Implémenter séquence 2 cercle (parse rayon).
- [ ] 2.4 Ajouter séquence 2 à l’interface web (champ Rayon + bouton).
- [ ] 2.5 Intégrer IMU LSM6DS3.

## Phase 3 — Robustesse, calibration et qualité (~3h)
- [ ] 3.1 Refaire triangle (séq.3) géométrique.
- [ ] 3.2 Unifier déclinaison magnétique.
- [ ] 3.3 Timeout sécurité WiFi.
- [ ] 3.4 Consolider facteur correction moteur (valeur unique).
- [ ] 3.5 Anti-windup amélioré PID.

## Phase 4 — Soutenance & rapport (~2h)
- [ ] 4.1 Teleplot mode final.
- [ ] 4.2 Tests unitaires minimum (3 courbes).
- [ ] 4.3 Nettoyer structure du dépôt (supprimer Graph/Code/ dupliqués).
- [ ] 4.4 Bonus : FA1 carrés circonscrits.

