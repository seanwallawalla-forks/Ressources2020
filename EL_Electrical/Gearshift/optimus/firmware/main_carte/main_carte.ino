/**************************************************************************/
/*!
    @file     main_carte.cpp
    @author   Bruno Moreira Nabinger and Corentin Lepais (EPSA)
                                                  Ecurie Piston Sport Auto
    
    Main code for control the Motor with integrated Controller and CAN 
    interface BG 45 CI

 
    @section  HISTORY
    v0.6 - 06/04/2019 Add the sending of the speed to the DTA
    v0.5 - 13/02/2019 Modifications linked to the class CAN
    v0.4 - 06/02/2019 Add fonction to can_interface
    v0.3 - 10/11/2018 Comments of code and add of the file 
           "projectconfig.h", with the definitions of the pins numbers.
    v0.2 - 17/10/2018 Management of pallets and beginning of creations of 
                      the associated functions
    v0.1 - 10/10/2018 First release (previously code of Pedro)

    example of code version comment
    v0.2 - Rewrote driver for Adafruit_Sensor and Auto-Gain support, and
           added lux clipping check (returns 0 lux on sensor saturation)
*/
/**************************************************************************/
#include "projectconfig.h"

//Ajout des fonctions
#include "fonct_palette_homing.h"
#include "fonct_mot.h"
#include "can_interface.h"

//initialisation Canbus
#include <SPI.h>  // ¿¿Importation redondante?? __ARS

//Definition of key-word
#define ERREUR -1 //Use to transmit that the motor is in error

//Definition of used variables 
boolean statePaletteIncrease; 
boolean statePaletteIncreaseBefore;
boolean statePaletteDecrease;
boolean statePaletteDecreaseBefore;

int positionEngager; // Contain what motor position is currently engaged
int wantedPosition;// Contain the motor position wanted so the speed rapport of the bike

boolean stateHoming; // Will contain the state of the homing button
boolean stateHomingBefore;

boolean outMotor1; //Info return by the motor
boolean outMotor2;//Info return by the motor

boolean stateNeutre;
boolean stateNeutreBefore;

boolean error;

const int neutrePosition = 2;
const int homingPosition=1;

boolean positionReached=true;

unsigned long T_D_Millis; //Containt the passed time

//Table which will contain the combination of the motor input for each speed
boolean motorPosition[16][4];//We use only 4 input motor to command it. The 5 is always 0

//Initialization of CANBUS
can_interface CAN;

void setup() 
{ 
  //Initialization of the pins
  pinMode(motorState1, INPUT);
  pinMode(motorState2, INPUT);
  pinMode(motorInput0, OUTPUT);
  pinMode(motorInput1, OUTPUT);
  pinMode(motorInput2, OUTPUT);
  pinMode(motorInput3, OUTPUT);
  pinMode(motorInput4, OUTPUT);
  pinMode(shiftCut, OUTPUT); 
  pinMode(gearPot, OUTPUT);

  pinMode(paletteIncrease, INPUT_PULLUP);  //  !! Il me semble qu'il y a deja des résistance de tirages sur le shield pour les pallettes !!__ARS
  pinMode(paletteDecrease, INPUT_PULLUP);  
  pinMode(neutre, INPUT_PULLUP);
  
  digitalWrite(motorInput0, LOW); // Initialisation de la position du moteur sur "Clear error and Stop" ¿Pourquoi? __ARS
  digitalWrite(motorInput1, LOW);
  digitalWrite(motorInput2, LOW);
  digitalWrite(motorInput3, LOW);
  digitalWrite(motorInput4, LOW);
  
  digitalWrite(shiftCut, HIGH); //Pas d'injection moteur

  //Initialization of the variables
  statePaletteIncreaseBefore = HIGH; //The pallets mode is INPUT_PULLUP, so the pin level is HIGH when it is inactive
  statePaletteDecreaseBefore = HIGH;
  positionEngager = 2;
  wantedPosition = 2;
  error = false;
  stateNeutreBefore=HIGH;
  
  
  T_D_Millis=millis();  //   ¿¿¿Variable de stckage tu temps???  __ARS
  
  {//Initialization of the table. We use only the position 1-6, clear error and start Homing
    
    //Clear error and Stop
    motorPosition[0][0] = 0;
    motorPosition[0][1] = 0;
    motorPosition[0][2] = 0;
    motorPosition[0][3] = 0; 

    //Start Homing
    motorPosition[1][0] = 1;
    motorPosition[1][1] = 0;
    motorPosition[1][2] = 0;
    motorPosition[1][3] = 0;

    //Position 1: Neutre
    motorPosition[2][0] = 0;
    motorPosition[2][1] = 1;
    motorPosition[2][2] = 0;
    motorPosition[2][3] = 0;

    //Position 2 : vitesse 1 de la moto
    motorPosition[3][0] = 1;
    motorPosition[3][1] = 1;
    motorPosition[3][2] = 0;
    motorPosition[3][3] = 0;

    //Position 3 : vitesse 2
    motorPosition[4][0] = 0;
    motorPosition[4][1] = 0;
    motorPosition[4][2] = 1;
    motorPosition[4][3] = 0;

    //Position 4 : vitesse 3
    motorPosition[5][0] = 1;
    motorPosition[5][1] = 0;
    motorPosition[5][2] = 1;
    motorPosition[5][3] = 0;

    //Position 5 : vitesse 4
    motorPosition[6][0] = 0;
    motorPosition[6][1] = 1;
    motorPosition[6][2] = 1;
    motorPosition[6][3] = 0;

    //Position 6 : vitesse 5
    motorPosition[7][0] = 1;
    motorPosition[7][1] = 1;
    motorPosition[7][2] = 1;
    motorPosition[7][3] = 0;

    //Position 7 : vitesse 6
    motorPosition[8][0] = 0;
    motorPosition[8][1] = 0;
    motorPosition[8][2] = 0;
    motorPosition[8][3] = 1;

    //Position 8
    motorPosition[9][0] = 1;
    motorPosition[9][1] = 0;
    motorPosition[9][2] = 0;
    motorPosition[9][3] = 1;

    //Position 9
    motorPosition[10][0] = 0;
    motorPosition[10][1] = 1;
    motorPosition[10][2] = 0;
    motorPosition[10][3] = 1;

    //Position 10
    motorPosition[11][0] = 1;
    motorPosition[11][1] = 1;
    motorPosition[11][2] = 0;
    motorPosition[11][3] = 1;

    //Position 11
    motorPosition[12][0] = 0;
    motorPosition[12][1] = 0;
    motorPosition[12][2] = 1;
    motorPosition[12][3] = 1;
    
    //Position 12
    motorPosition[13][0] = 1;
    motorPosition[13][1] = 0;
    motorPosition[13][2] = 1;
    motorPosition[13][3] = 1;

    //Position 13 
    motorPosition[14][0] = 0;
    motorPosition[14][1] = 1;
    motorPosition[14][2] = 1;
    motorPosition[14][3] = 1;

    //Position 14
    motorPosition[15][0] = 1;
    motorPosition[15][1] = 1;
    motorPosition[15][2] = 1;
    motorPosition[15][3] = 1;
  }

}

void loop() 
{ 
  CAN.Recieve();//MAJ of the can attributs by recieving the last datas
  
  //Control of pallet+
  statePaletteIncrease = digitalRead(paletteIncrease);
  //If we only test if the state of the palette have changed or if we only test if the state is now 1, the test is going to be true 2 times:
  // when the pilot presses the palette and when it is released. We want to change the speed only  one time for each press

  //0 -> 1 = Detect the rising edge of signal statePaletteIncrease;
  if (statePaletteIncrease != statePaletteIncreaseBefore)// Check if state have changed
  {
    if (!statePaletteIncrease) // Check if state changed to 1, so we have the rising edge 0 -> 1 
    {
      if(PassageVitesseIsPossible(positionEngager)) 
      {
        digitalWrite(shiftCut, LOW); //Stop injection
        wantedPosition = positionEngager+1;
      }
    }
    statePaletteIncreaseBefore = statePaletteIncrease;
  }
  
  //Control of pallet-
  statePaletteDecrease = digitalRead(paletteDecrease);
  //If we only test if the state of the palette have changed or if we only test if the state is now 1, the test is going to be true 2 times:
  // when the pilot presses the palette and when it is released. We want to change the speed only  one time for each press
  
  //0 -> 1 = Detect the rising edge of signal statePaletteDecrease;
  if (statePaletteDecrease != statePaletteDecreaseBefore)// Check if state have changed
  {
    if (!statePaletteDecrease) // Check if state changed to 1, so we have the rising edge 0 -> 1
    {
      if(PassageVitesseIsPossible(positionEngager))
      {
        digitalWrite(shiftCut, LOW); //Stop injection 
        wantedPosition = positionEngager-1;
      }
    }
    statePaletteIncreaseBefore = statePaletteIncrease;
  }

  //Gestion du neutre
  stateNeutre = digitalRead(neutre);
  if(stateNeutre != stateNeutreBefore)
  {
    if(!stateNeutre)
    {
      digitalWrite(shiftCut, LOW); 
      wantedPosition = neutrePosition;
    }
    stateNeutreBefore = !stateNeutre;
  }
  
  //Gestion bouton homing
  stateHoming=CAN.getHomingState(); //We have the state of the homing thank to the can attribut
  if(stateHoming != stateHomingBefore)
  {
    if(!stateHoming)
    {
      wantedPosition=homingPosition;
    }
    stateHomingBefore = !stateHomingBefore;
  }

  if (wantedPosition!=positionEngager) //We try to change speed only if the pilot demands it
  {
    digitalWrite(shiftCut, LOW);//Close the injection
    EngageVitesse(wantedPosition);
    outMotor1 = digitalRead(motorState1);
    outMotor2 = digitalRead(motorState2);
    while(MotorIsTurning(outMotor1,outMotor2)) //while the motor is turning
    {
      outMotor1 = digitalRead(motorState1);
      outMotor2 = digitalRead(motorState2);
      positionReached=true; //We guess that we will reach the correct position
      if (MotorIsLost(outMotor1,outMotor2)) //error
      {
        //We clean the error
        EngageVitesse(0); //the motor stop
        //We transmit the error to the CAN
        while(!CAN.Transmit(ERREUR, T_D_Millis)) 
        {
          //We try to transmit the error until the transmission is good
        }
        T_D_Millis=millis(); // We save the time of last transmit
        positionReached=false;
      }
    }
    
    if (positionReached) //We change the current speed only if we have reached the position
    {
      positionEngager=wantedPosition; //We save the engaged position
    }
    digitalWrite(shiftCut, HIGH);//Open the injection
  }
  
  if (CAN.Transmit(positionEngager-2, T_D_Millis)); //We sent the engaged speed to the CAN (Speed= PositionEngager-2)
  {
     T_D_Millis=millis(); // We save the time of last transmit
     TransmetToDTATheGear(positionEngager-2); // We send to the DTA the engaged gear
  }
}

void EngageVitesse(int wantedPosition) //Function which pass the speed
{
  digitalWrite(motorInput0,LOW);
  digitalWrite(motorInput1, motorPosition[wantedPosition][0]);
  digitalWrite(motorInput2, motorPosition[wantedPosition][1]);
  digitalWrite(motorInput3, motorPosition[wantedPosition][2]);
  digitalWrite(motorInput4, motorPosition[wantedPosition][3]);
}

void TransmetToDTATheGear(int rapportEngager)
{
  long valAnalog[7];
  for(int i=0;i<7;i++)
  {
    valAnalog[i]=0.2+0.8*i; //mappage des valeurs de tensions envoyés au DTA en fonction de la vitesse (0->0.2V .... 6->5V)
  }
  analogWrite(gearPot,valAnalog[rapportEngager]);
}
