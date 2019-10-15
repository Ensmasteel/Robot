#ifndef PID_H_
#define PID_H_
#define NBPROFILES 5
#define TIMEBLOCKED 0.2
#include "Filtre.h"
#include "Vector.h"

struct PIDProfile
{
    float KP, KI, KD;
    float IRelax;
    float epsilon, dEpsilon;
    float maxErr;
};

PIDProfile newPIDProfile(float KP, float KI, float KD, float IRelax, float epsilon, float dEpsilon, float maxErr);

// La variable x définit une grandeur quelconque. dx est sa derivee.

class PID
{
private:
    PIDProfile PIDProfiles[NBPROFILES];
    int8_t currentProfile;
    float iTerm;
    bool modulo360;
    Filtre dxF;
    float timeBlocked;

public:
    bool close;
    bool blocked;
    void reset();
    void setPIDProfile(uint8_t id, PIDProfile pidProfile);
    void setCurrentProfile(uint8_t id);
    float compute(float xTarget, float dxTarget, float x, float dx, float dt);
    PID(bool modulo360, float frequency);
    PID();
};

class Asservissement
{
public:
    PID pidTranslation;
    PID pidRotation;
    bool close;
    bool blocked;
    bool needToGoForward;
    void compute(float *outTranslation, float *outRotation, Cinetique cRobot, Cinetique cGhost, float dt);
    Asservissement(float frequency);
};

#endif