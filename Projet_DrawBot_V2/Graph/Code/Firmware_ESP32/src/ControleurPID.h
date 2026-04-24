#ifndef CONTROLEUR_PID_H
#define CONTROLEUR_PID_H

class ControleurPID {
public:
    ControleurPID(double kp, double ki, double kd, double outputLimit);
    void setConsigne(double consigne);
    double calculer(double valeurActuelle);
    void reset();

    void setKp(double kp);
    void setKi(double ki);
    void setKd(double kd);

    double getKp() const;
    double getKi() const;
    double getKd() const;
    double getConsigne() const;

private:
    double _kp, _ki, _kd;
    double _consigne;
    double _erreurPrecedente;
    double _integraleErreur;
    unsigned long _tempsPrecedentMillis; // Renommé pour clarté
    double _outputLimit;
};

#endif // CONTROLEUR_PID_H