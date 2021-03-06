#include "Actuators.h"

//test git

Actuator::Actuator(String name, MessageID messID)
{
    this->messID = messID;
    this->name = name;
    etat = Actuator_State::Attente;
}

Actuator_State Actuator::Update()
{
    return etat;
}

void Actuator::NewOrder(Actuator_Order order)
{
    currentOrder = order;
    etat = Actuator_State::NewMess;
}

/**Pavillon::Pavillon() : Actuator("Pav", MessageID::Pavillon_M)
{
    stepperMotor = new StepperMotorJ(pinStep, pinDir, pinSleep, pinM0, pinM1);
}

void Pavillon::Init(uint8_t pinDir, uint8_t pinStep, uint8_t pinSleep, uint8_t pinM0, uint8_t pinM1)
{
    this->pinDir = pinDir;
    this->pinStep = pinStep;
    this->pinSleep = pinSleep;
    this->pinM0 = pinM0;
    this->pinM1 = pinM1;
    etat = Actuator_State::Attente;
}

Actuator_State Pavillon::Update()
{   
    /*Serial.println("Pavillon");
    Serial.println(int(etat));
    Serial.println(String(currentOrder));
    Serial.print("etat :");
    Serial.println(int(etat));
    switch (etat)
    {
    case Actuator_State::NewMess:
        switch (currentOrder)
        {
        case Actuator_Order::Monter:
            stepperMotor->move(actionStep,500,true,false);
            break;

        case Actuator_Order::Descendre:
            stepperMotor->move(actionStep,500,false,false);
            break;

        default:
            break;
        }
        etat = Actuator_State::MouvFinished;
        break;
    
    case Actuator_State::MouvFinished:
        etat = Actuator_State::Attente;
        break;

    default:
        break;
    }

   return Actuator::Update();
}*/


// A utiliser avec la fonction sendAction et pas sendOrderAction
Tourelle::Tourelle() : Actuator("Tourelle")
{

}

void Tourelle::Init(uint8_t pinDir, uint8_t pinStep, uint8_t pinM0, uint8_t pinM1, MessageID ID)
{
    messID = ID;
    switch (messID)
    {
    case MessageID::TourelleD_M:
        name += "D";
        break;

    case MessageID::TourelleG_M:
        name += "G";
        break;
    
    default:
        break;
    }
    this->pinDir = pinDir;
    this->pinStep = pinStep;
    this->pinM0 = pinM0;
    this->pinM1 = pinM1;
    stepperMotor = new StepperMotorJ(pinDir, pinStep, pinM0, pinM1);
    etat = Actuator_State::Attente;
}

void Tourelle::setAngleTourelleVoulu(int angle){
    this->angleTourelleVoulu=angle;
}

int Tourelle::calculateStepsByAngle(int angle){
    float rapport = 3;
    return round(rapport*angle/1.8);
}

Actuator_State Tourelle::Update()
{
    switch (etat)
    {
    case Actuator_State::NewMess:
        switch (currentOrder)
        {
        case Actuator_Order::Tourner:
            step=calculateStepsByAngle(abs(angleTourelle-angleTourelleVoulu));
            Logger::debugln("steps :" + (String) step);
            if(angleTourelle>angleTourelleVoulu){
                stepperMotor->move(step,4000,true,false);
            }
            else{
                stepperMotor->move(step,4000,false,false);
            }
            
            Logger::debugln("angleT :" + (String) angleTourelle);
            Logger::debugln("angleTV :" + (String) angleTourelleVoulu);
            angleTourelle=angleTourelleVoulu;
            Logger::debugln("angleTF :" + (String) angleTourelle);
            break;

        /*case Actuator_Order::TournerAntiHoraire:
            step=calculateStepsByAngle(angleTourelle-angleTourelleVoulu);
            stepperMotor->move(step,2000,false,false);
            angleTourelle=angleTourelleVoulu;
            break;
        */
        default:
            break;
        }
        etat = Actuator_State::MouvFinished;
        break;

    case Actuator_State::MouvFinished:
        etat = Actuator_State::Attente;
        break;

    default:
        break;
    }
    return Actuator::Update();
}

Pompe::Pompe() : Actuator("Pompe"){

}

void Pompe::Init(uint8_t pinMOSFET,MessageID ID){
    messID=ID;
    switch(messID){
        case MessageID::Pompe_BrasD_M:
            name+="BrasD";
            break;
        
        case MessageID::Pompe_BrasG_M:
            name+="BrasG";
            break;
        
        case MessageID::Pompe_StockageD_M:
            name+="StockageD";
            break;
        
        case MessageID::Pompe_StockageG_M:
            name+="StockageG";
            break;

        default:
            break;
    }

    this->pinMOSFET=pinMOSFET;
    pinMode(pinMOSFET,OUTPUT);
    etat = Actuator_State::Attente;
}

Actuator_State Pompe::Update()
{
    switch (etat)
    {
    case Actuator_State::NewMess:
        switch (currentOrder)
        {
        case Actuator_Order::ActiverPompe:
            Logger::debugln("actif");
            digitalWrite(pinMOSFET,HIGH);
            break;

        case Actuator_Order::DesactiverPompe:
            Logger::debugln("desactif");
            digitalWrite(pinMOSFET,LOW);
            break;

        default:
            break;
        }
        etat = Actuator_State::MouvFinished;
        break;

    case Actuator_State::MouvFinished:
        etat = Actuator_State::Attente;
        break;

    default:
        break;
    }
    return Actuator::Update();   
}

uint8_t Pompe::getState()
{
    return digitalRead(pinMOSFET);
}

PositionBras::PositionBras()
{

}

PositionBras PositionBras::Init(int posServo1,int posServo2,int posServo3){
    this->posServo1=posServo1;
    this->posServo2=posServo2;
    this->posServo3=posServo3;
    return *this;
}  

PositionBras &PositionBras::operator=(const PositionBras &source){
    posServo1=source.posServo1;
    posServo2=source.posServo2;
    posServo3=source.posServo3;
    return *this;
}

int PositionBras::getPosServo1(){
    return posServo1;
}

int PositionBras::getPosServo2(){
    return posServo2;
}

int PositionBras::getPosServo3(){
    return posServo3;
}


Bras::Bras() : Actuator("Bras")
{
    this->posRepos.Init(30,60,90);
    this->posStockagePalet.Init(20,30,15);
    this->posStockagePalet2.Init(40,10,7);
    this->posPaletSol.Init(105,130,138);
    this->posPaletDistributeur.Init(0,0,0);
    this->posPaletStatuette = posPaletStatuette.Init(0,0,0);
    this->posRamassageStatuette = posRamassageStatuette.Init(90,88,85);
    this->posDepotStatuette = posDepotStatuette.Init(0,0,0);
    this->posStockageStatuette = posStockageStatuette.Init(90,20,90);
    this->posDepotReplique = posDepotReplique.Init(0,0,0);
    this->posDepotPaletGallerieB = posDepotPaletGallerieB.Init(45,90,180);
    this->posDepotPaletGallerieH = posDepotPaletGallerieH.Init(0,0,0);

    this->posIntermediaire = posIntermediaire.Init(80,70,80);
}

Actuator_State Bras::Update()
{
    switch (etat)
    {
    case Actuator_State::NewMess:
        switch (currentOrder)
        {
        case Actuator_Order::PositionRepos:
            servo1.write(posRepos.getPosServo1()+this->erreurS1);
            servo3.write(posRepos.getPosServo3()+this->erreurS3);
            delay(200);
            servo2.write(posRepos.getPosServo2()+this->erreurS2);
            break;

        case Actuator_Order::PositionStockagePalet:
            servo1.write(posStockagePalet.getPosServo1()+this->erreurS1);
            delay(200);
            servo2.write(posStockagePalet.getPosServo2()+this->erreurS2);
            servo3.write(posStockagePalet.getPosServo3()+this->erreurS3);
            break;

        case Actuator_Order::PositionPaletSol:
            servo1.write(posPaletSol.getPosServo1()+this->erreurS1);
            delay(200);
            servo2.write(posPaletSol.getPosServo2()+this->erreurS2);
            servo3.write(posPaletSol.getPosServo3()+this->erreurS3);
            break;        
        
        case Actuator_Order::PositionPaletDistributeur:
            servo1.write(posPaletDistributeur.getPosServo1()+this->erreurS1);
            delay(200);
            servo2.write(posPaletDistributeur.getPosServo2()+this->erreurS2);
            servo3.write(posPaletDistributeur.getPosServo3()+this->erreurS3);
            break;

        case Actuator_Order::PositionPaletStatuette:
            servo1.write(posPaletStatuette.getPosServo1()+this->erreurS1);
            delay(200);
            servo2.write(posPaletStatuette.getPosServo2()+this->erreurS2);
            servo3.write(posPaletStatuette.getPosServo3()+this->erreurS3);
            break;

        case Actuator_Order::PositionRamassageStatuette:
            servo1.write(posRamassageStatuette.getPosServo1()+this->erreurS1);
            delay(200);
            servo2.write(posRamassageStatuette.getPosServo2()+this->erreurS2);
            servo3.write(posRamassageStatuette.getPosServo3()+this->erreurS3);
            break;

        case Actuator_Order::PositionDepotStatuette:
            servo1.write(posDepotStatuette.getPosServo1()+this->erreurS1);
            delay(200);
            servo2.write(posDepotStatuette.getPosServo2()+this->erreurS2);
            servo3.write(posDepotStatuette.getPosServo3()+this->erreurS3);
            break;

        case Actuator_Order::PositionStockageStatuette:
            servo1.write(posStockageStatuette.getPosServo1()+this->erreurS1);
            delay(200);
            servo2.write(posStockageStatuette.getPosServo2()+this->erreurS2);
            servo3.write(posStockageStatuette.getPosServo3()+this->erreurS3);
            break;

        case Actuator_Order::PositionDepotReplique:
            servo1.write(posDepotReplique.getPosServo1()+this->erreurS1);
            delay(200);
            servo2.write(posDepotReplique.getPosServo2()+this->erreurS2);
            servo3.write(posDepotReplique.getPosServo3()+this->erreurS3);
            break;

        case Actuator_Order::PositionEchange:
            servo1.write(posEchange.getPosServo1()+this->erreurS1);
            delay(200);
            servo2.write(posEchange.getPosServo2()+this->erreurS2);
            servo3.write(posEchange.getPosServo3()+this->erreurS3);
            break;

        case Actuator_Order::PositionDepotPaletGallerieB:
            servo1.write(posDepotPaletGallerieB.getPosServo1()+this->erreurS1);
            delay(200);
            servo2.write(posDepotPaletGallerieB.getPosServo2()+this->erreurS2);
            servo3.write(posDepotPaletGallerieB.getPosServo3()+this->erreurS3);
            break;

        case Actuator_Order::PositionDepotPaletGallerieH:
            servo1.write(posDepotPaletGallerieH.getPosServo1()+this->erreurS1);
            delay(200);
            servo2.write(posDepotPaletGallerieH.getPosServo2()+this->erreurS2);
            servo3.write(posDepotPaletGallerieH.getPosServo3()+this->erreurS3);
            break;

        default:
            break;
        }
        etat = Actuator_State::MouvFinished;
        break;
    
    case Actuator_State::MouvFinished:
        etat = Actuator_State::Attente;
        break;
        
    default:
        break;
    }

    return Actuator::Update();
}

void Bras::Init(uint8_t pinServo1, uint8_t pinServo2, uint8_t pinServo3, MessageID ID)
{
    messID = ID;
    switch (messID)
    {
    case MessageID::BrasD_M:
        name += "D";
        this->erreurS1=-3.0;
        this->erreurS2=-5.0;
        this->erreurS3=1.0;

        break;

    case MessageID::BrasG_M:
        name += "G";
        this->erreurS1=6.0;
        this->erreurS2=-4.0;
        this->erreurS3=-2.0;
        break;
    
    default:
        break;
    }

    this->pinServo1 = pinServo1; 
    this->pinServo2 = pinServo2; 
    this->pinServo3 = pinServo3;

    this->posStart=posStart;


    servo1.attach(pinServo1);
    servo2.attach(pinServo2);
    servo3.attach(pinServo3);

    servo1.write(posStockageStatuette.getPosServo1() + erreurS1);
    servo2.write(posStockageStatuette.getPosServo2() + erreurS2);
    servo3.write(posStockageStatuette.getPosServo3() + erreurS3);

    etat = Actuator_State::Attente;
}


/*Pince::Pince() : Actuator("Pince")
{
}

void Pince::Init(uint8_t pinServo, uint8_t pinDir, uint8_t pinStep, uint8_t pinSleep, uint8_t pinM0, uint8_t pinM1, MessageID ID, int ferme, int ouvert, int actionStep)
{
    messID = ID;
    switch (messID)
    {
    case MessageID::PinceArr_M:
        name += "Arr";
        break;

    case MessageID::PinceAvD_M:
        name += "AvD";
        break;

    case MessageID::PinceAvG_M:
        name += "AvG";
        break;
    
    default:
        break;
    }

    this->pinServo = pinServo; 
    this->pinDir = pinDir;
    this->pinStep = pinStep;
    this->pinSleep = pinSleep;
    this->pinM0 = pinM0;
    this->pinM1 = pinM1;
    posFermee = ferme;
    posOuverte = ouvert;
    this-> actionStep = actionStep;

    servo.attach(pinServo);
    servo.write(posFermee);

    stepperMotor = new StepperMotorJ(pinStep, pinDir, pinM0, pinM1);

    etat = Actuator_State::Attente;
}

Actuator_State Pince::Update()
{
    switch (etat)
    {
    case Actuator_State::NewMess:
        switch (currentOrder)
        {
        case Actuator_Order::Monter:
            stepperMotor->move(actionStep,standardDelay,true,true);
            break;

        case Actuator_Order::Descendre:
            stepperMotor->move(actionStep,standardDelay,false,true);
            break;
        
        case Actuator_Order::Ouvrir:
            servo.write(posOuverte);
            break;
        
        case Actuator_Order::Fermer:
            servo.write(posFermee);
            break;

        default:
            break;
        }
        etat = Actuator_State::MouvFinished;
        break;
    
    case Actuator_State::MouvFinished:
        etat = Actuator_State::Attente;
        break;

    default:
        break;
    } 

    return Actuator::Update();
}

Actuator_State PinceAvant::Update()
{
    switch (etat)
    {
    case Actuator_State::NewMess:
        switch (currentOrder)
        {
        
        case Actuator_Order::Stock:
            servo.write(posOuverte);
            stepperMotor->move(miniStep, standardDelay, true, true);//rapide
            servo.write(posFermee);
            delay(200);
            stepperMotor->move(miniStep+actionStep, standardDelay, false, true); //rapide
            stepperMotor->move(miniStep, standardDelay*2, false, true); //lent
            delay(200);
            servo.write(posTresOuverte);
            stepperMotor->move(miniStep+actionStep, standardDelay, true, true);//rapide
            break;

        case Actuator_Order::Destock:
            stepperMotor->move(actionStep, standardDelay, false, true);//rapide
            stepperMotor->move(miniStep, standardDelay*2, false, true);//lent
            delay(200);
            servo.write(posFermee);
            delay(200);
            stepperMotor->move(miniStep*2, standardDelay*3, true, true);//tres lent
            stepperMotor->move(actionStep-miniStep, standardDelay, true, true);//rapide
            servo.write(posTresOuverte);
            break;

        default:
            break;
        }

    default:
        break;
    } 

    return Pince::Update();
}

Actuator_State PinceArriere::Update()
{
    switch (etat)
    {
    case Actuator_State::NewMess:
        switch (currentOrder)
        {
            case Actuator_Order::Stock:
            //depuis la position haute, ouvre les pince puis descend legerment, ferme les pinces et remonte
            servo.write(posOuverte);
            stepperMotor->move(actionStep, standardDelay, false, true);//rapide descente
            servo.write(posFermee);
            stepperMotor->move(actionStep, standardDelay*3, true, true); //tres lente montee
            break;

        case Actuator_Order::Destock:
            stepperMotor->move(actionStep, standardDelay, false, true);//rapide descente
            servo.write(posOuverte);
            stepperMotor->move(actionStep, standardDelay*2, true, true); //lente montee
            break;

        default:
            break;
        }

    default:
        break;
    } 

    return Pince::Update();
}*/