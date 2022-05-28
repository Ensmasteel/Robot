#include "Robot.h"
#include "Arduino.h"
#include "Actions.h"
#include "Functions.h"
#include "Codeuse.h"

#define PIN_CODEUSE_GAUCHE_A 29
#define PIN_CODEUSE_GAUCHE_B 28
#define PIN_CODEUSE_DROITE_A 34
#define PIN_CODEUSE_DROITE_B 35

#define PIN_MOTEUR_GAUCHE_PWR 2
#define PIN_MOTEUR_GAUCHE_SENS 24
#define PIN_MOTEUR_GAUCHE_BRAKE 37

#define PIN_MOTEUR_DROITE_PWR 3
#define PIN_MOTEUR_DROITE_SENS 25
#define PIN_MOTEUR_DROITE_BRAKE 39

#define PIN_INTERRUPTEUR_ARR_DROITE 30
#define PIN_INTERRUPTEUR_ARR_GAUCHE 33

#define ELOIGNEMENT_CODEUSES 0.275
#define DIAMETRE_ROUE_CODEUSE_DROITE 0.0532
#define DIAMETRE_ROUE_CODEUSE_GAUCHE 0.0535
#define TICKS_PER_ROUND 16384

#define SKIP_TELEMETRY_LONG 10000
#define SKIP_TELEMETRY_FAST 50

Robot::Robot(float xIni, float yIni, float thetaIni, TeamColor tc, Stream *commPort, Stream *actuPort, Stream *espPort)
{
    this->espPort = espPort;//=====================================

    MoveProfiles::setup();
    cinetiqueCurrent = Cinetique(xIni, yIni, thetaIni);
    odometrie = Odometrie(TICKS_PER_ROUND, &cinetiqueCurrent, ELOIGNEMENT_CODEUSES, PIN_CODEUSE_GAUCHE_A, PIN_CODEUSE_GAUCHE_B, 
                          DIAMETRE_ROUE_CODEUSE_GAUCHE, PIN_CODEUSE_DROITE_A, PIN_CODEUSE_DROITE_B, DIAMETRE_ROUE_CODEUSE_DROITE, 
                          PIN_INTERRUPTEUR_ARR_DROITE, PIN_INTERRUPTEUR_ARR_GAUCHE);

    motorLeft = Motor(PIN_MOTEUR_GAUCHE_PWR, PIN_MOTEUR_GAUCHE_SENS, 12);
    pinMode(PIN_MOTEUR_DROITE_BRAKE, OUTPUT);
    digitalWrite(PIN_MOTEUR_DROITE_BRAKE, LOW); //Adaptation ancien driver
    motorRight = Motor(PIN_MOTEUR_DROITE_PWR, PIN_MOTEUR_DROITE_SENS, 12);
    pinMode(PIN_MOTEUR_GAUCHE_BRAKE, OUTPUT);
    digitalWrite(PIN_MOTEUR_GAUCHE_BRAKE, LOW); //Adaptation ancien driver

    ghost = Ghost(cinetiqueCurrent);
    controller = Asservissement(&translationOrderPID, &rotationOrderPID, &cinetiqueCurrent, &cinetiqueNext, filterFrequency);
    communication = Communication(commPort);
    commActionneurs = Communication(actuPort);
    this->setTeamColor(tc);


    sequences = new Sequence*[__NBSEQUENCES__]; //On définit un array de la bonne taille
    Action::setPointer(this);
    for (int i=0;i<__NBSEQUENCES__;i++)
        sequences[i] = new Sequence(i);

    //ATTENTION, LES ACTIONS DOIVENT ETRE DEFINIE EN TANT QUE ROBOT BLEU !
    // Might be define in main.cpp->setup

    TargetVector base = TargetVector(0.150,1.435,false);
    TargetVector baseJ = TargetVector(2.90,1.435,false);
    TargetVector northBase = TargetVector(0.22,0.35,false);
    TargetVector southBase = TargetVector(0.22,1.3,false);

    TargetVector paletVCote = TargetVector(-0.65,1.7,false);
    TargetVector paletBCote = TargetVector(0.121,0.312,false);
    TargetVector paletRCote = TargetVector(0.312,0.120,false);
    //TargetVector gobeletV2 = TargetVector(0.300,0.800,false);

    TargetVector paletBCentreCache = TargetVector(0.900,1.445,false);
    TargetVector paletRCentreCache = TargetVector(0.900,1.205,false);
    //TargetVector paletVCentreCache = TargetVector(0.830,1.325,true);
    TargetVector paletVCentreCacheJ = TargetVector(2.170,1.325,false);
    //TargetVector gobeletV4 = TargetVector(1.270,0.800,false);
    
    TargetVector paletZoneBas = TargetVector(0.800,0.625,false); //par rapport a ou vous regardez les peintres (pour le nom)
    TargetVector paletZoneHaut = TargetVector(1.150,0.625,false);
    TargetVector paletZoneGauche = TargetVector(0.975,0.800,false); //Enfin sauf la c'est inverse en fonction du cote
    TargetVector paletZoneDroite = TargetVector(0.975,0.450,false);
    
    TargetVector paletsDistributeur = TargetVector(0.121,0.750,false);
    TargetVector devantGallerie = TargetVector(0.830,1.700,false);
    TargetVector devantGallerieJ = TargetVector(2.170,1.700,false);
    
    TargetVector vitrine = TargetVector(0.225,2.000,false);
    TargetVector statuette = TargetVector(0.255,0.250,false);

    Sequence* mainSequence = getSequenceByName(mainSequenceName);
        Serial.println("entree dans main");
        //TODO config recalage etc

        //Attend le message Tirette
        //mainSequence->add(new Send_Order_Action(BrasD_M, Actuator_Order::PositionStockageStatuette, 10.0, &commActionneurs, true));
        if(teamColor==BLEU){
            mainSequence->add(new Send_Order_Action(Pompe_BrasG_M, Actuator_Order::ActiverPompe, -1,&commActionneurs, true));
        }
        else{
            mainSequence->add(new Send_Order_Action(Pompe_BrasD_M, Actuator_Order::ActiverPompe, -1,&commActionneurs, true));
        }
        mainSequence->add(new Wait_Tirette_Action(33));
        mainSequence->add(new Do_Action(startTimeSeq));
        /*Cote jaune*/
        if(teamColor==BLEU){

        TargetVector tmp = TargetVector(0.250,1.370,false);
        mainSequence->add(new StraightTo_Action( -1,tmp, standard));
        mainSequence->add(new StraightTo_Action(-1,TargetVector(0.90,0.400,false),standard));
        mainSequence->add(new Spin_Action( -1,TargetVectorE(-PI/2, false), standard));
        if(teamColor==BLEU){
            mainSequence->add(new Send_Order_Action(BrasD_M, Actuator_Order::PositionRamassageStatuette, 10.0, &commActionneurs, true));
        }
        else{
            mainSequence->add(new Send_Order_Action(BrasG_M, Actuator_Order::PositionRamassageStatuette, 10.0, &commActionneurs, true));
        }
        mainSequence->add(new Forward_Action(-1,0.100,standard));
        mainSequence->add(new Backward_Action(-1,0.100,standard));
        mainSequence->add(new StraightTo_Action( -1,TargetVector(0.680,0.575, false), standard));
        if (teamColor==BLEU){
            mainSequence->add(new Send_Order_Action(Pompe_BrasD_M, Actuator_Order::ActiverPompe, -1,&commActionneurs, true));
        }
        else{
            mainSequence->add(new Send_Order_Action(Pompe_BrasG_M, Actuator_Order::ActiverPompe, -1,&commActionneurs, true));
        }       
        mainSequence->add(new Spin_Action(5, TargetVectorE(-3*PI/4, false), standard));
        mainSequence->add(new Forward_Action(-1,0.270,standard));
        if (teamColor==BLEU){
            mainSequence->add(new Send_Order_Action(BrasD_M, Actuator_Order::PositionStockageStatuette, 10.0, &commActionneurs, true));
        }
        else{
            mainSequence->add(new Send_Order_Action(BrasG_M, Actuator_Order::PositionStockageStatuette, 10.0, &commActionneurs, true));
        }
          mainSequence->add(new Spin_Action(5, TargetVectorE(-3*PI/4 - 0.65, false), standard));
        mainSequence->add(new Backward_Action(-1,0.040,standard));
        if (teamColor==BLEU){
            mainSequence->add(new Send_Order_Action(BrasG_M, Actuator_Order::PositionRamassageStatuette, 10.0, &commActionneurs, true));
        }
        else{
            mainSequence->add(new Send_Order_Action(BrasD_M, Actuator_Order::PositionRamassageStatuette, 10.0, &commActionneurs, true));
        }
        mainSequence->add(new Sleep_Action(1));
        if (teamColor==BLEU){
            mainSequence->add(new Send_Order_Action(Pompe_BrasG_M, Actuator_Order::DesactiverPompe, -1,&commActionneurs, true));
        }
        else{
            mainSequence->add(new Send_Order_Action(Pompe_BrasD_M, Actuator_Order::DesactiverPompe, -1,&commActionneurs, true));
        }
        
        mainSequence->add(new Backward_Action(-1,0.100,standard));
        if (teamColor==BLEU){
            mainSequence->add(new Send_Order_Action(BrasG_M, Actuator_Order::PositionRepos, 10.0, &commActionneurs, true));
        }
        else{
            mainSequence->add(new Send_Order_Action(BrasD_M, Actuator_Order::PositionRepos, 10.0, &commActionneurs, true));
        }
        
        mainSequence->add(new StraightTo_Action( -1,TargetVector(0.150,1.50, false), standard));
        mainSequence->add(new StraightTo_Action( -1,TargetVector(0.100,1.75, false), standard));
        if (teamColor==BLEU){
            mainSequence->add(new Send_Order_Action(BrasD_M, Actuator_Order::PositionRamassageStatuette, 10.0, &commActionneurs, true));
            mainSequence->add(new Send_Order_Action(Pompe_BrasD_M, Actuator_Order::DesactiverPompe, -1,&commActionneurs, true));
            mainSequence->add(new Send_Order_Action(BrasD_M, Actuator_Order::PositionRepos, 10.0, &commActionneurs, true));
        }
        else{
            mainSequence->add(new Send_Order_Action(BrasG_M, Actuator_Order::PositionRamassageStatuette, 10.0, &commActionneurs, true));
            mainSequence->add(new Send_Order_Action(Pompe_BrasG_M, Actuator_Order::DesactiverPompe, -1,&commActionneurs, true));
            mainSequence->add(new Send_Order_Action(BrasG_M, Actuator_Order::PositionRepos, 10.0, &commActionneurs, true));            
        }
        //mainSequence->add(new Send_Order_Action(BrasG_M, Actuator_Order::PositionStockagePalet2, 10.0, &commActionneurs, true));
        
        mainSequence->add(new StraightTo_Action( -1,TargetVector(0.570,1.575, false), standard));
        mainSequence->add(new Send_Order_Action(BrasD_M, Actuator_Order::PositionPaletSol, 10.0, &commActionneurs, true));
        mainSequence->add(new Send_Order_Action(BrasG_M, Actuator_Order::PositionPaletSol, 10.0, &commActionneurs, true));
        mainSequence->add(new Send_Order_Action(Pompe_BrasD_M, Actuator_Order::ActiverPompe, -1,&commActionneurs, true));
        mainSequence->add(new Send_Order_Action(Pompe_BrasG_M, Actuator_Order::ActiverPompe, -1,&commActionneurs, true));
        mainSequence->add(new Send_Order_Action(BrasD_M, Actuator_Order::PositionRepos, 10.0, &commActionneurs, true));
        mainSequence->add(new Send_Order_Action(BrasG_M, Actuator_Order::PositionRepos, 10.0, &commActionneurs, true));
        mainSequence->add(new Spin_Action( -1,TargetVectorE(0, false), standard));
         mainSequence->add(new Spin_Action( -1,TargetVectorE(PI/4, false), standard));
         mainSequence->add(new Spin_Action( -1,TargetVectorE(PI/2, false), standard));
        //mainSequence->add(new Forward_Action(-1,0.400,standard));
        mainSequence->add(new Send_Order_Action(BrasD_M, Actuator_Order::PositionDepotPaletGallerieB, 10.0, &commActionneurs, true));
        mainSequence->add(new Send_Order_Action(BrasG_M, Actuator_Order::PositionDepotPaletGallerieB, 10.0, &commActionneurs, true));
        mainSequence->add(new Forward_Action(-1,0.240,standard));
        mainSequence->add(new Send_Order_Action(Pompe_BrasD_M, Actuator_Order::DesactiverPompe, -1,&commActionneurs, true));
        mainSequence->add(new Send_Order_Action(Pompe_BrasG_M, Actuator_Order::DesactiverPompe, -1,&commActionneurs, true));
        mainSequence->add(new Send_Order_Action(BrasD_M, Actuator_Order::PositionRepos, 10.0, &commActionneurs, true));
        mainSequence->add(new Send_Order_Action(BrasG_M, Actuator_Order::PositionRepos, 10.0, &commActionneurs, true));
        mainSequence->add(new Backward_Action(-1,0.100,standard));
        }


        else{   // if teamColor == JAUNE (donc que l'on est bleu, logique (ironie, merci nos corrections de bug a la va vite))
            
        TargetVector tmp = TargetVector(0.250,1.370,false);
        mainSequence->add(new StraightTo_Action( -1,tmp, standard));
        mainSequence->add(new StraightTo_Action(-1,TargetVector(0.90,0.370,false),standard));
        mainSequence->add(new Spin_Action( -1,TargetVectorE(-PI/2, false), standard));
        if(teamColor==BLEU){
            mainSequence->add(new Send_Order_Action(BrasD_M, Actuator_Order::PositionRamassageStatuette, 10.0, &commActionneurs, true));
        }
        else{
            mainSequence->add(new Send_Order_Action(BrasG_M, Actuator_Order::PositionRamassageStatuette, 10.0, &commActionneurs, true));
        }
        mainSequence->add(new Forward_Action(-1,0.100,standard));
        mainSequence->add(new Backward_Action(-1,0.100,standard));
        mainSequence->add(new StraightTo_Action( -1,TargetVector(0.661,0.606, false), standard));
        if (teamColor==BLEU){
            mainSequence->add(new Send_Order_Action(Pompe_BrasD_M, Actuator_Order::ActiverPompe, -1,&commActionneurs, true));
        }
        else{
            mainSequence->add(new Send_Order_Action(Pompe_BrasG_M, Actuator_Order::ActiverPompe, -1,&commActionneurs, true));
        }       
        mainSequence->add(new Spin_Action(5, TargetVectorE(-3*PI/4, false), standard));
        mainSequence->add(new Forward_Action(-1,0.285,standard));
        if (teamColor==BLEU){
            mainSequence->add(new Send_Order_Action(BrasD_M, Actuator_Order::PositionStockageStatuette, 10.0, &commActionneurs, true));
        }
        else{
            mainSequence->add(new Send_Order_Action(BrasG_M, Actuator_Order::PositionStockageStatuette, 10.0, &commActionneurs, true));
        }
          mainSequence->add(new Spin_Action(5, TargetVectorE(-3*PI/4 - 0.65, false), standard));
        mainSequence->add(new Backward_Action(-1,0.040,standard));
        if (teamColor==BLEU){
            mainSequence->add(new Send_Order_Action(BrasG_M, Actuator_Order::PositionRamassageStatuette, 10.0, &commActionneurs, true));
        }
        else{
            mainSequence->add(new Send_Order_Action(BrasD_M, Actuator_Order::PositionRamassageStatuette, 10.0, &commActionneurs, true));
        }
        mainSequence->add(new Sleep_Action(1));
        if (teamColor==BLEU){
            mainSequence->add(new Send_Order_Action(Pompe_BrasG_M, Actuator_Order::DesactiverPompe, -1,&commActionneurs, true));
        }
        else{
            mainSequence->add(new Send_Order_Action(Pompe_BrasD_M, Actuator_Order::DesactiverPompe, -1,&commActionneurs, true));
        }
        mainSequence->add(new Backward_Action(-1,0.100,standard));
        if (teamColor==BLEU){
            mainSequence->add(new Send_Order_Action(BrasG_M, Actuator_Order::PositionRepos, 10.0, &commActionneurs, true));
        }
        else{
            mainSequence->add(new Send_Order_Action(BrasD_M, Actuator_Order::PositionRepos, 10.0, &commActionneurs, true));
        }
        
        mainSequence->add(new StraightTo_Action( -1,TargetVector(0.27,1.50, false), standard));
        mainSequence->add(new StraightTo_Action( -1,TargetVector(0.27,1.75, false), standard));
        if (teamColor==BLEU){
            mainSequence->add(new Send_Order_Action(BrasD_M, Actuator_Order::PositionRamassageStatuette, 10.0, &commActionneurs, true));
            mainSequence->add(new Send_Order_Action(Pompe_BrasD_M, Actuator_Order::DesactiverPompe, -1,&commActionneurs, true));
            mainSequence->add(new Send_Order_Action(BrasD_M, Actuator_Order::PositionRepos, 10.0, &commActionneurs, true));
        }
        else{
            mainSequence->add(new Send_Order_Action(BrasG_M, Actuator_Order::PositionRamassageStatuette, 10.0, &commActionneurs, true));
            mainSequence->add(new Send_Order_Action(Pompe_BrasG_M, Actuator_Order::DesactiverPompe, -1,&commActionneurs, true));
            mainSequence->add(new Send_Order_Action(BrasG_M, Actuator_Order::PositionRepos, 10.0, &commActionneurs, true));            
        }
        //mainSequence->add(new Send_Order_Action(BrasG_M, Actuator_Order::PositionStockagePalet2, 10.0, &commActionneurs, true));
        //mainSequence->add(new Spin_Action( -1,TargetVectorE(PI/6, false), standard));
        //mainSequence->add(new Spin_Action( -1,TargetVectorE(-PI/6, false), standard));
        mainSequence->add(new StraightTo_Action( -1,TargetVector(0.66,1.52, false), standard));
        mainSequence->add(new Send_Order_Action(BrasD_M, Actuator_Order::PositionPaletSol, 10.0, &commActionneurs, true));
        mainSequence->add(new Send_Order_Action(BrasG_M, Actuator_Order::PositionPaletSol, 10.0, &commActionneurs, true));
        mainSequence->add(new Send_Order_Action(Pompe_BrasD_M, Actuator_Order::ActiverPompe, -1,&commActionneurs, true));
        mainSequence->add(new Send_Order_Action(Pompe_BrasG_M, Actuator_Order::ActiverPompe, -1,&commActionneurs, true));
        mainSequence->add(new Send_Order_Action(BrasD_M, Actuator_Order::PositionRepos, 10.0, &commActionneurs, true));
        mainSequence->add(new Send_Order_Action(BrasG_M, Actuator_Order::PositionRepos, 10.0, &commActionneurs, true));
        mainSequence->add(new Spin_Action( -1,TargetVectorE(PI/2 * 0.98, false), standard));
        //mainSequence->add(new Forward_Action(-1,0.400,standard));
        mainSequence->add(new Send_Order_Action(BrasD_M, Actuator_Order::PositionDepotPaletGallerieB, 10.0, &commActionneurs, true));
        mainSequence->add(new Send_Order_Action(BrasG_M, Actuator_Order::PositionDepotPaletGallerieB, 10.0, &commActionneurs, true));
        mainSequence->add(new Forward_Action(-1,0.260,standard));
        mainSequence->add(new Send_Order_Action(Pompe_BrasD_M, Actuator_Order::DesactiverPompe, -1,&commActionneurs, true));
        mainSequence->add(new Send_Order_Action(Pompe_BrasG_M, Actuator_Order::DesactiverPompe, -1,&commActionneurs, true));
        mainSequence->add(new Send_Order_Action(BrasD_M, Actuator_Order::PositionRepos, 10.0, &commActionneurs, true));
        mainSequence->add(new Send_Order_Action(BrasG_M, Actuator_Order::PositionRepos, 10.0, &commActionneurs, true));
        mainSequence->add(new Backward_Action(-1,0.450,standard));

        }






        mainSequence->add(new End_Action(false,false));
        Serial.println("avant StartSequence");
        mainSequence->startSelected();
    
    Serial.println("mainpass");
        //déclenchée par timeSequence
    Sequence* goNorth = getSequenceByName(goNorthName);
        goNorth->add(new StraightTo_Action(-1,base,standard));
        goNorth->add(new End_Action());
        goNorth->pause(false);//Cette action ne doit pas se lancer dès le début

        //déclenchée par timeSequence
    Sequence* goSouth = getSequenceByName(goSouthName);
        goSouth->add(new StraightTo_Action(-1,southBase,standard));
        goSouth->add(new End_Action());
        goSouth->pause(false); //Cette action ne doit pas se lancer dès le début

    Sequence* communicationSequence = getSequenceByName(communicationSequenceName);

        Switch_Message_Action* messageSwitch = new Switch_Message_Action(-1,&communication,NO_REQUIREMENT);
        messageSwitch->addPair(MessageID::Empty_M , ping);
        messageSwitch->addPair(MessageID::Em_Stop_M,shutdown);
        messageSwitch->addPair(MessageID::PID_tweak_M,PID_tweak);
        messageSwitch->addPair(MessageID::North_M,setNorth);
        messageSwitch->addPair(MessageID::South_M,setSouth);

        communicationSequence->add(messageSwitch);
        communicationSequence->add(new End_Action(true,false));
        communicationSequence->startSelected();

        //déclenchée par mainSequence juste après la tirette
    Sequence* timeSequence = getSequenceByName(timeSequenceName);
        timeSequence->add(new Do_Action(setTimeStart));
        timeSequence->add(new Sleep_Action(85));
        timeSequence->add(new Do_Action(startBackHomeSeq));
        timeSequence->add(new Sleep_Action(15));
        timeSequence->add(new Do_Action(shutdown));
        timeSequence->add(new End_Action());
        timeSequence->pause(true); //La time sequence ne doit s'écouler qu'a partir du tiré de la tirette !!

    Sequence* recallageListener = getSequenceByName(recallageListerName);
        recallageListener->add(new Wait_Error_Action(PID_FAIL_ERROR,14.0));
        recallageListener->add(new Do_Action(recallageBordure,-1));

        //Si cette action s'active, elle s'active au plus tard 14s après le démarrage du backward.
        //Sachant qu'on a un timeout de 15s sur le backward, on est sur que la seule action qu'on peut forcer c'est le backward
        recallageListener->add(new Do_Action(forceMainSeqNext,-1)); 
        recallageListener->add(new End_Action(true,true,false));
        recallageListener->pause(false);

    ghost.Lock(false);
}

void Robot::Update_Cinetique(float dt)
{   //Logger::debugln("Update_Cinetique");
    odometrie.updateCinetique(dt);
    /*Logger::debugln("odometrie : ");
    Logger::debugln(String(odometrie.codeuseDroite.ticks));
    Logger::debugln(String(odometrie.codeuseGauche.ticks));*/
}

void Robot::Update(float dt)
{   
    /*
    //================= communication with esp ==========
    while(this->espPort->available()){
        char c= this->espPort->read();
        if ( c != '\n')
        {
            readString += c;
        }
        else
        {
            if(readString[0] == 'f'){
                readString.remove(0);
                rangeAdversaryFoward = readString.toInt();
            }
            else if (readString[0] == 'b')
            {
                readString.remove(0);
                rangeAdversaryBackward = readString.toInt();
            }
            readString = "";
        }
    }
    //================= communication with esp ==========
    */

    communication.update();
    commActionneurs.update();

    if(stopped){//no mater if the robot move fowar/backard, stop if an obstacle
        //Logger::debugln("go in the if ??");
        motorLeft.stop();
        motorRight.stop();
        Update_Cinetique(dt);
        motorLeft.actuate();
        motorRight.actuate();
        Logger::debugln("STOP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    }
    else if (dem){
        motorLeft.resume();
        motorRight.resume();
        Update_Cinetique(dt);
        motorLeft.actuate();
        motorRight.actuate();
        dem=false;
        Logger::debugln("Demarrage!!!!!!!!!!!!!!!!!!!!!!!!!!");
    }
    /*else if(dem){
        motorLeft.resume();
        motorRight.resume();
        Update_Cinetique(dt);
        motorLeft.actuate();
        motorRight.actuate();
    }*/
    else{
        //Logger::debugln("update");
        Update_Cinetique(dt);
        //Logger::debugln("ghost");
        ghost.ActuatePosition(dt);
        cinetiqueNext = ghost.Get_Controller_Cinetique();
        controller.compute(dt);
        
        //Logger::debugln("translation OrderPID :" + String(translationOrderPID));
        //Logger::debugln("rotation OrderPID :" + String(rotationOrderPID));
        
        //================= recalage ==========
        //if (odometrie.getInterGaucheContact()) {
            motorLeft.setOrder(-(translationOrderPID - rotationOrderPID));
            motorLeft.actuate();
        //}
        //if (odometrie.getInterDroiteContact()) {
            motorRight.setOrder(translationOrderPID + rotationOrderPID);
            motorRight.actuate();
        //}
        //================= recalage ==========
        
        for (int i=0; i<__NBSEQUENCES__;i++){
            sequences[i]->update();
        }

        
    }

    /*
    other variante
    if((rangeAdversaryFoward<100 && translationOrderPID>0) || (rangeAdversaryBackward<100 && translationOrderPID>0))
    {
        motorLeft.stop();
        motorRight.stop();
        stopped = true;
    }
    else
    {
        the code
    }
    */
    
    if (compteur==SKIP_TELEMETRY_LONG){
        telemetry(false,true);
        compteur=0;
    } else if (compteur%SKIP_TELEMETRY_FAST==0){
        telemetry(true,false);
    }
    if (communication.inWaitingRx() > 0)
        communication.popOldestMessage(); //Tout le monde a eu l'occasion de le peek, on le vire.
    if (commActionneurs.inWaitingRx() > 0)
        commActionneurs.popOldestMessage(); //Tout le monde a eu l'occasion de le peek, on le vire.
    if (ErrorManager::inWaiting() > 0)
        ErrorManager::popOldestError(); //Les séquences ont eu l'occasion de le lire, on le vire
    compteur++;
}

void Robot::telemetry(bool odometrie, bool other)
{
    if (odometrie)
    {
        cinetiqueCurrent.toTelemetry("R");
        ghost.Get_Controller_Cinetique().toTelemetry("G");
    }
    if (other)
    {
        Logger::toTelemetry("pid", String(controller.close));
        Logger::toTelemetry("ghost", String(ghost.trajectoryIsFinished()));
        getSequenceByName(mainSequenceName)->toTelemetry();
        communication.toTelemetry();
    }
}

Sequence* Robot::getSequenceByName(SequenceName name){
    return sequences[(int)name];
}

float Robot::getTime(){
    return millis()/1e3 - timeStarted;
}

void Robot::setTeamColor(TeamColor teamColor){
    this->teamColor=teamColor;
    Target::setTeamColor(teamColor);
}

TeamColor Robot::getTeamColor(){
    return teamColor;
}

void Robot::move(VectorE where)
{
    cinetiqueCurrent = Cinetique(where._x,where._y,where._theta,0,0);
    cinetiqueNext = cinetiqueCurrent;
    ghost.moveGhost(where);
}

void Robot::move(TargetVectorE whereTarget){
    move(whereTarget.getVectorE());
}

void Robot::recalibrateGhost()
{
    ghost.moveGhost(cinetiqueCurrent);
}

Vector Robot::approcheElementbyVector(Vector depart,Vector arrivee){
    float x = arrivee._x - cos(calculateTheta(depart,arrivee)) * dist_arrivee;
    float y = arrivee._y - sin(calculateTheta(depart,arrivee)) * dist_arrivee;
    Logger::debugln("x : " + (String) x);
    Logger::debugln("y : " + (String) y);
    return Vector(x,y);
}