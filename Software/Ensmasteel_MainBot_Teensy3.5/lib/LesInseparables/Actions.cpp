#include "Actions.h"
#include "Robot.h"
#include "Sequence.h"

//========================================ACTION GENERIQUES========================================
Robot *Action::robot;

void Action::setPointer(Robot *robot_){
    robot=robot_;
}

void Action::start()
{
    timeStarted=millis()/1e3; started=true;
}

bool Action::hasFailed()
{
    if (timeout < 0)
        return false;
    return millis() / 1e3 > timeStarted + timeout;
}

void Double_Action::doAtEnd(){
    action2->doAtEnd();
}

void Double_Action::start()
{
    action1->start();
    Action::start();
}

bool Double_Action::isFinished()
{
    if (action2->hasStarted())
        return action2->isFinished();
    else //On s'occupe de action1
    {
        if (action1->isFinished()) //Il faut passer à 2
        {
            action1->doAtEnd();
            action2->start();
        }
        return false;
    }
}

bool Double_Action::hasFailed()
{
    if (action2->hasStarted())
        return action2->hasFailed();
    else //On s'occupe de action1
        return action1->hasFailed();
}

Double_Action::Double_Action(float timeout, String name,int16_t require) : Action(name, timeout, require)
{
    this->action1 = nullptr;
    this->action2 = nullptr;
}

//========================================ACTION MOVES========================================

void Move_Action::start()
{
    robot->recalibrateGhost();
    robot->controller.setCurrentProfile(profileName);
    int err;
    err = robot->ghost.Compute_Trajectory(posFinal, deltaCurve,MoveProfiles::get(profileName,!pureRotation)->speedRamps , MoveProfiles::get(profileName,!pureRotation)->cruisingSpeed, pureRotation, backward);
    if (err == 0)
        Logger::debugln("Computation succeeded");
    else
        Logger::infoln("Computation failed");
    robot->ghost.Lock(false);
    Action::start();
}

void Move_Action::doAtEnd()
{
    robot->controller.sendScoreToTelemetry();
    robot->controller.reset();
}

bool Move_Action::isFinished()
{
    return robot->ghost.trajectoryIsFinished() && robot->controller.close;
}

bool Move_Action::hasFailed()
{
    return /*asser->tooFar ||*/ Action::hasFailed();
}

Move_Action::Move_Action(float timeout, VectorE posFinal, float deltaCurve, MoveProfileName profileName, bool pureRotation, bool backward, String name, int16_t require) : Action(name, timeout, require)
{
    this->posFinal = posFinal;
    this->deltaCurve = deltaCurve;
    this->profileName = profileName;
    this->pureRotation = pureRotation;
    this->backward = backward;
}

Goto_Action::Goto_Action(float timeout, TargetVectorE target, float deltaCurve, MoveProfileName profileName, bool backward, int16_t require)
    : Move_Action(timeout, target.getVectorE(), deltaCurve, profileName, false, backward, "Goto", require)
{ /*Rien a faire d'autre*/
}

Spin_Action::Spin_Action(float timeout, TargetVectorE target, MoveProfileName profileName, int16_t require)
    : Move_Action(timeout, target.getVectorE() , 0.0, profileName, true, false, "Spin", require) //x et y seront modifié par start
{                                                                                    /*Rien a faire d'autre*/
}

void Spin_Action::start()
{
    posFinal._x = robot->cinetiqueCurrent._x;
    posFinal._y = robot->cinetiqueCurrent._y;
    Move_Action::start();
}

Rotate_Action::Rotate_Action(float timeout, float deltaTheta, MoveProfileName profileName, int16_t require)
    : Move_Action(timeout, VectorE(0.0, 0.0, 0.0), 0.0, profileName, true, false, "Rota", require) //x et y et theta seront modifié par start
{
    this->deltaTheta=deltaTheta;
}

void Rotate_Action::start()
{
    posFinal._x = robot->cinetiqueCurrent._x;
    posFinal._y = robot->cinetiqueCurrent._y;
    posFinal._theta = robot->cinetiqueCurrent._theta + deltaTheta;
    Move_Action::start();
}


Forward_Action::Forward_Action(float timeout, float dist, MoveProfileName profileName, int16_t require)
    : Move_Action(timeout, VectorE(0.0, 0.0, 0.0), 0.0, profileName, false, false, "Forward", require)
{
    this->dist = dist;
}

void Forward_Action::start()
{
    posFinal._theta = robot->cinetiqueCurrent._theta;
    posFinal._x = (robot->cinetiqueCurrent._x) + dist * cos(normalizeAngle(posFinal._theta));
    posFinal._y = (robot->cinetiqueCurrent._y) + dist * sin(normalizeAngle(posFinal._theta));
    Move_Action::start();
}

Backward_Action::Backward_Action(float timeout, float dist, MoveProfileName profileName, int16_t require)
    : Move_Action(timeout, VectorE(0.0, 0.0, 0.0), 0.0, profileName, false, true, "Backward", require)
{
    this->dist = dist;
}

void Backward_Action::start()
{
    posFinal._theta = robot->cinetiqueCurrent._theta;
    posFinal._x = (robot->cinetiqueCurrent._x) - dist * cos(normalizeAngle(posFinal._theta));
    posFinal._y = (robot->cinetiqueCurrent._y) - dist * sin(normalizeAngle(posFinal._theta));
    Move_Action::start();
}

void StraightTo_Action::start()
{
    //X et Y sont déja miroiré à ce moment. 
    Vector delta = Vector(x, y) - robot->cinetiqueCurrent;
    float cap = delta.angle();
    spin = new Spin_Action(timeout, TargetVectorE(cap,true), profileName);  //Donc il faut etre en absolu
    goTo = new Goto_Action(timeout, TargetVectorE(x,y,cap,true), 0.1, profileName);
    action1 = spin;
    action2 = goTo;
    Double_Action::start();
}

StraightTo_Action::StraightTo_Action(float timeout, TargetVector target, MoveProfileName profileName, int16_t require) : Double_Action(timeout, "stTo", require)
{
    Vector targetV = target.getVector();
    this->x = targetV._x;
    this->y = targetV._y;
    this->profileName = profileName;
    this->timeout = timeout;
}

Brake_Action::Brake_Action(float timeout, int16_t require) : Move_Action(timeout,VectorE(0,0,0),0.1,brake,false,false,"brak",require){}
//========================================ACTION COMM========================================

Send_Action::Send_Action(Message message, int16_t require) : Action("Send", 0.1, require)
{
    this->message = message;
}

void Send_Action::start()
{
    robot->communication.send(message);
    done = true;
    Action::start();
}

Wait_Message_Action::Wait_Message_Action(MessageID messageId, float timeout, int16_t require) : Action("WaitMess", timeout, require)
{
    this->messageId = messageId;
}

bool Wait_Message_Action::isFinished()
{
    return robot->communication.inWaitingRx() > 0 && extractID(robot->communication.peekOldestMessage()) == messageId;
}

Switch_Message_Action::Switch_Message_Action(float timeout,int16_t require) : Action("swch",timeout,require)
{
    this->doFct.clear();
    this->onMessage.clear();
    size=0;
}

void Switch_Message_Action::addPair(MessageID messageId,Fct fct)
{
    this->onMessage.push_back(messageId);
    this->doFct.push_back(fct);
    size++;
}

bool Switch_Message_Action::isFinished()
{
    if (robot->communication.inWaitingRx() > 0)
    {
        for (int i=0;i<size;i++)
        {
            if (extractID(robot->communication.peekOldestMessage()) == onMessage[i])
            {
                doFct[i](robot); //Les functions agissent sur la mainSequence uniquement
                return true;
            }
        }
    }
    return false;
}

//========================================ACTION MISC========================================

End_Action::End_Action(bool loop, bool pause) : Action("End_", -1, NO_REQUIREMENT)
{
    this->loop=loop;
    this->pause=pause;
}

void End_Action::start()
{
    //Au premier appel de start, done == false et on met la sequence en pause
    //Sinon, done == true et alors on va looper.
    if (pause && !done)
        mySequence->pause();

    if (loop)
    {
        mySequence->nextIndex=0; //Une end action, boucle sa propre sequence
        done=true;
    }
    Action::start();
}

void Do_Action::start()
{
    functionToCall(robot); //Les functions agissent sur la mainSequence uniquement
    done=true;
    Action::start();
}

Sleep_Action::Sleep_Action(float timeToWait,int16_t require) : Action("ZZzz",-1,require)
{
    this->timeToWait=timeToWait;
}

bool Sleep_Action::isFinished()
{
    return millis()/1e3 - timeStarted>timeToWait;
}

Wait_Error_Action::Wait_Error_Action(Error error, float timeout, int16_t require) : Action("WaitErr", timeout, require){
    this->error = error;
}

bool Wait_Error_Action::isFinished(){
    return ErrorManager::inWaiting() > 0 && ErrorManager::peekOldestError() == error;
}

PauseSeq_Action::PauseSeq_Action(SequenceName nameSeq, int16_t require) : Action("paus",0.1,require){
    this->nameSeq=nameSeq;
}

void PauseSeq_Action::start(){
    robot->getSequenceByName(nameSeq)->pause();
    done=true;
    Action::start();
}

ResumeSeq_Action::ResumeSeq_Action(SequenceName nameSeq, int16_t require) : Action("resu",0.1,require){
    this->nameSeq=nameSeq;
}

void ResumeSeq_Action::start(){
    robot->getSequenceByName(nameSeq)->resume();
    done=true;
    Action::start();
}

/*
* /!\ Le timeout specifié dans la sequence d'ecoute "recallageListner" doit etre plus petit que celui ci
*/
Recallage_Action::Recallage_Action(bool arriere, float dist, float timeout) : Double_Action(timeout)
{
    action1 = new ResumeSeq_Action(recallageListerName);
    if (arriere)
        action2 = new Backward_Action(timeout,dist,recallage);
    else
        action2 = new Forward_Action(timeout,dist,recallage);
}