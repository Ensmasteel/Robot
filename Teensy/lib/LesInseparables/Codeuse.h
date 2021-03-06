#ifndef CODEUSE_H_
#define CODEUSE_H_

#include "Vector.h"
#include "Arduino.h"

#ifndef STM32BOTH
#include <Encoder.h>
#else // !STM32BOTH

/**
 *Defines a new class Encoder.
 */

class Encoder
{
public:
    Encoder(uint8_t pinA, uint8_t pinB) {}
    int32_t read() { return 0; }
};
#endif

/**
 * Defines a class for a switch.
 */
class Interrupteur
{
    private:
        uint8_t pin;
        bool contact;
    public:
        Interrupteur(uint8_t pin);
        void updateContact();
        bool isContact();
};

/**
 * Defines a class for coder containing all parameters.
 */
class Codeuse
{
    int32_t oldTicks; //Nombre de ticks compté, nombre de ticks compté au dernier appel
    float debug;             //Distance parcourue par la codeuse depuis l'alllumage
    float diametreRoue;
    double ticksPerRound; //Nombre de ticks par tours de roue
    Encoder *enc;           //Objet Encoder de la librairie Encoder.

public:
    float v, deltaAvance; //Vitesse et avance du robot AU NIVEAU DE LA ROUE CODEUSE
    int32_t ticks;
    void actuate(float dt); //Actualise (transforme les ticks en vitesse puis en avance)
    Codeuse();
    Codeuse(uint8_t pinA, uint8_t pinB, double ticksPerRound, float diametreRoue);
};

/**
 * Creates a class for odometry.
 */
class Odometrie
{
private:
    //Codeuse codeuseGauche, codeuseDroite;
    Interrupteur *interGauche, *interDroite;
    Cinetique *cinetique;
    float eloignementCodeuses;

public:
    Codeuse codeuseGauche, codeuseDroite;
    void updateCinetique(float dt);
    Odometrie();
    Odometrie(double ticksPerRound, Cinetique *cinetique, float eloignementCodeuses,
              uint8_t pinACodeuseGauche, uint8_t pinBCodeuseGauche, float diametreRoueGauche,
              uint8_t pinACodeuseDroite, uint8_t pinBCodeuseDroite, float diametreRoueDroite,
              uint8_t pinInterDroite, uint8_t pinInterGauche);
    bool getInterDroiteContact();
    bool getInterGaucheContact();
};


#endif