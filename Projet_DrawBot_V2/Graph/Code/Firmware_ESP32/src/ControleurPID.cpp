#include "ControleurPID.h"
#include <Arduino.h> // Pour millis()

ControleurPID::ControleurPID(double kp, double ki, double kd, double outputLimit)
    : _kp(kp), _ki(ki), _kd(kd), _consigne(0),
      _erreurPrecedente(0), _integraleErreur(0), _tempsPrecedentMillis(0), _outputLimit(outputLimit) {}

void ControleurPID::setConsigne(double consigne) {
    _consigne = consigne;
    // Il est souvent bon de réinitialiser le PID quand la consigne change drastiquement
    // ou après une longue période d'inactivité, surtout le terme intégral.
    reset(); 
}

double ControleurPID::calculer(double valeurActuelle) {
    unsigned long tempsActuelMillis = millis();
    // Gérer le premier appel ou si le temps n'a pas avancé (ou a bouclé pour millis())
    if (_tempsPrecedentMillis == 0 || tempsActuelMillis <= _tempsPrecedentMillis) {
         _tempsPrecedentMillis = tempsActuelMillis;
         // Pour le premier calcul, on pourrait retourner seulement P*erreur ou 0
         // Ici, on retourne 0 pour éviter un pic de dérivée si dt est considéré nul.
         // Ou calculer P si on veut une réaction immédiate: return _kp * (_consigne - valeurActuelle);
         return 0.0; 
    }

    double dt_secondes = (tempsActuelMillis - _tempsPrecedentMillis) / 1000.0; 
    if (dt_secondes <= 0.0001) { // dt trop petit, éviter division par zéro ou instabilités
        // On peut choisir de ne pas mettre à jour _tempsPrecedent pour forcer un dt plus grand au prochain appel,
        // ou retourner la dernière sortie ou une sortie basée sur P seulement.
        // Pour un PID de position, une erreur persistante doit quand même être traitée par I.
        double erreur_simple = _consigne - valeurActuelle;
         _integraleErreur += erreur_simple * dt_secondes; // dt_secondes est petit, donc l'impact sur I est petit
        double sortie_simple = (_kp * erreur_simple) + (_ki * _integraleErreur);
        // Limiter la sortie
        if (sortie_simple > _outputLimit) sortie_simple = _outputLimit;
        else if (sortie_simple < -_outputLimit) sortie_simple = -_outputLimit;
        return sortie_simple;
    }

    double erreur = _consigne - valeurActuelle;
    _integraleErreur += erreur * dt_secondes;
    
    // Anti-windup simple pour le terme intégral
    if (_ki != 0.0) { // Éviter division par zéro si _ki est nul ou pour ne pas limiter inutilement
        double maxTermeIntegral = _outputLimit / (abs(_ki) + 0.00001); // Limite basée sur la sortie max et Ki
        if (_integraleErreur > maxTermeIntegral) _integraleErreur = maxTermeIntegral;
        if (_integraleErreur < -maxTermeIntegral) _integraleErreur = -maxTermeIntegral;
    }


    double deriveeErreur = (erreur - _erreurPrecedente) / dt_secondes;
    double sortie = (_kp * erreur) + (_ki * _integraleErreur) + (_kd * deriveeErreur);

    _erreurPrecedente = erreur;
    _tempsPrecedentMillis = tempsActuelMillis;

    // Limiter la sortie totale du PID
    if (sortie > _outputLimit) sortie = _outputLimit;
    else if (sortie < -_outputLimit) sortie = -_outputLimit;
    
    return sortie;
}

void ControleurPID::reset() {
    _integraleErreur = 0;
    _erreurPrecedente = 0;
    _tempsPrecedentMillis = millis(); // Important pour le calcul de dt à la prochaine itération
                                   // Mettre à 0 pour forcer le cas initial dans calculer() peut aussi être une option.
}

void ControleurPID::setKp(double kp) { _kp = kp; }
void ControleurPID::setKi(double ki) { _ki = ki; if (abs(ki) < 0.00001) _integraleErreur = 0.0; } // Reset l'intégrale si Ki devient nul
void ControleurPID::setKd(double kd) { _kd = kd; }

double ControleurPID::getKp() const { return _kp; }
double ControleurPID::getKi() const { return _ki; }
double ControleurPID::getKd() const { return _kd; }
double ControleurPID::getConsigne() const { return _consigne; }