#ifndef SEQUENCES_H
#define SEQUENCES_H

// S1 : escalier.
// Avancer 20 cm, gauche 90 deg, avancer 10 cm, droite 90 deg, avancer 40 cm.
void sequenceEscalier();

// S2 : cercle simple. Le rayon est approxime par le ratio roue interieure.
// radius_cm est garde pour l'interface et les futurs reglages fins.
void sequenceCircle(int radius_cm = 20);

// S3 : fleche orientee Nord, sequence de test propre sans capteurs.
void sequenceNorthArrow();

#endif // SEQUENCES_H
