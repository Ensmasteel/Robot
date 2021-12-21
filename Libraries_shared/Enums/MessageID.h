#ifndef MESSAGEID_H_
#define MESSAGEID_H_
#include <stdint.h>

#define DROIT_byteM 0
#define GAUCHE_byteM 1

// COMPLETER LES ENUMERATIONS PAR LA FIN //


// En-tete de message //
///////////////////////
/**
 * This file defines the enumerations usefull to defines the actuators states and message IDs.
 */

/**
 * This enum contains all the possible MessageID. This enum has to be updated every year before the cup.
 * Each documentation of a member of this enum has same structure :
 * [the sender > the receptioner] : the action described | the data contained in the type of message (optional)
 */
enum MessageID : uint16_t
{
    Empty_M,        ///< Empty message
    Stop_M,         ///< [Teensy -> Aux]: Stops the actuators
    Tirette_M,      ///< [Mega -> Teensy]:   zipper pulled over.
    PID_tweak_M,    ///< [Aux -> Teensy]:    Change PID | DATA: 4 bytes [bool incr, bool translation, wichOne (P=0,I=1,D=2)]
    add_forward_M,  ///<[Aux -> Teensy]:     Add a forward order at the end of the pincipal queue
    add_backward_M, ///<[Aux -> Teensy]:     Add a backward order at the end of the principal queue
    add_spin_M,     ///<[Aux -> Teensy]:     Add a spin order at the end of the principal queue
    Em_Stop_M,      ///<[Aux -> Teensy]:     Emergency Stop
    Evitemment_M,   ///<[Mega/Aux -> Teensy]:  There might be an obstacle | DATA: Vector (obstacle pos)
    North_M,        ///<[Camera -> Teensy]:   It must end at north [[deprecated in the 2022 edition]]
    South_M,        ///<[Camera -> Teensy]:   It must end at south [[deprecated in the 2022 edition]]
    Pavillon_M,     ///<[Teensy -> Mega]:     Rise the 'Pavillon'! | byte0 = Actuator_Order |
    BrasG_M,        ///<[Teensy -> Mega]:     Get the left arm IN or OUT  | byte0 = Actuator_Order |
    BrasD_M,        ///<[Teensy -> Mega]:     Get the right arm IN or OUT | byte0 = Actuator_Order |
    PinceAvG_M,     ///<[Teensy -> Mega]:     Ascend, descend, open or close the left front gripper | byte0 = Actuator_Order |
    PinceAvD_M,     ///<[Teensy -> Mega]:     Ascend, descend, open or close the right front gripper | byte0 = Actuator_Order |
    PinceArr_M      ///<[Teensy -> Mega]:     Ascend, descend, open or close the back gripper | byte0 = Actuator_Order |
};

// Complements d'ordres //
/////////////////////////

// Byte 0

/**
 * Enum containing the byte0 of each order to precise the exact action to do. To be adapt for each cup occurrence.
 *
 * Each entry of the enum will be documented this way : target actuator | description of action
 */
enum Actuator_Order : uint8_t
{
    Sortir,     ///< arm | getting out the arm
    Rentrer,    ///< arm | getting in the arm
    Monter,     ///<  Pavillon | Rails, ascend
    Descendre,  ///<  Pavillon | Rails, downing
    Stock,      ///<  Rails | assembly of action, getting a cup, ascending arm, downing it.
    Destock,    ///<  Rails | assembly of actions, ascending arm, getting cup, downing it, open gripper.
    Fermer,     ///<  Gripper | close it.
    Ouvrir      ///<  Gripper | open it.
};

#endif