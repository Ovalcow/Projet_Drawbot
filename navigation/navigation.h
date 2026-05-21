#ifndef NAVIGATION_H
#define NAVIGATION_H

#include "../config/config.h"

// Initialise la couche navigation (PID internes)
void setupNavigation();

// ═══════════════════════════════════════════════════════════════
//  Primitives de déplacement asservies
//  Chaque fonction est BLOQUANTE : elle ne retourne qu'une fois
//  le mouvement terminé.
// ═══════════════════════════════════════════════════════════════

// Avancer en ligne droite d'une distance donnée (cm)
// distance_cm > 0 : avance  |  distance_cm < 0 : recule
void avancerCm(float distance_cm);

// Tourner sur place d'un angle donné (degrés)
// angle_deg > 0 : tourne à droite  |  angle_deg < 0 : tourne à gauche
void tournerDeg(float angle_deg);

// Tracer un cercle de rayon donné (cm)
// Le robot revient (idéalement) au point de départ
void tracerCercle(float rayon_cm);

// Orienter le robot vers le Nord magnétique
// Utilise le magnétomètre pour trouver le cap
void orienterVersNord();

// ═══════════════════════════════════════════════════════════════
//  Primitives utilitaires
// ═══════════════════════════════════════════════════════════════

// Avancer en ligne droite pendant une durée (mode open-loop, fallback)
void avancerDuree(int direction, int pwm, unsigned long duration_ms);

// Tourner pendant une durée (mode open-loop, fallback)
void tournerDuree(int turn_dir, int pwm, unsigned long duration_ms);

// Marquer le point de départ / arrivée (petit trait perpendiculaire)
void marquerPoint();

#endif // NAVIGATION_H
