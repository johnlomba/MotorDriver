/*
 Name:		MotorControllerSlave.ino
 Created:	4/28/2022 7:38:40 AM
 Author:	jlomba
*/

#include <ezButton.h>
#include "FLEXOR.h"
#include <Wire.h>

#define I2C_SLAVE_ADDRESS 11 // address of board

#define PAYLOAD_SIZE 2 //Size of info being recieved

#define Pulse 9 //Pin being used to send pulse to stepper driver

#define Dir 11 //Pin being used to control direction of stepper driver

#define ENA 13 //pin being used to enable/disable stepper driver

#define ZERO 3 //pin being used to enable/disable stepper driver

#define END 2 //pin being used to enable/disable stepper driver

#define END_POWER 4 //pin being used to enable/disable stepper driver

#define ZERO_POWER 5 //pin being used to enable/disable stepper driver

FLEXORClass flexi(Pulse, Dir, ENA, ZERO, END, END_POWER, ZERO_POWER);

int n = 0; //Store Received byte info

void setup()
{
    flexi.Init();

    //Open I2C connection with address 11
    Wire.begin(I2C_SLAVE_ADDRESS); 

    //Setup functions to use when I2C information is received or requested.           
    Wire.onRequest(requestEvents);
    Wire.onReceive(receiveEvents);
}

void loop()
{
    if (flexi.IsEnabled()) 
    {
        flexi.Loop();
    }   
}


//used to send I2C data
void requestEvents()
{
    Serial.println(F("---> recieved request, sending 0"));
    Wire.write(0);
}

//Used to recieve I2C data
void receiveEvents(int numBytes)
{
    n = Wire.read();

    switch (n)
    {
    case 0: DisableMachine(); break;
    case 1: EnableMachine();  break;
    case 2: InitiateHoming(); break;
    case 3: IncreaseTravelDistance(); break;
    case 4: DecreaseTravelDistance(); break;
    case 5: IncreaseSpeed(); break;
    case 6: DecreaseSpeed(); break;
    default: break;
    }
}


//Start homing sequence
void InitiateHoming()
{
    flexi.StartHoming();
}

//Disables Driver
void DisableMachine()
{
    if (flexi.IsRunning())
    {
        flexi.IsRunning(false);
    }
    else
    {
        flexi.Enable(false);
    }  
}

//Enable Driver
void EnableMachine()
{
    if (!flexi.IsEnabled()) 
    {
        flexi.Enable(true);
    }
    else
    {
        flexi.IsRunning(true);
    } 
}

//Increase travel distance with user input
void IncreaseTravelDistance()
{
    flexi.IncreaseTravelDistance();
}

//Decrease travel distance with user input
void DecreaseTravelDistance()
{
    flexi.DecreaseTravelDistance();
}

//Increase Speed with user input
void IncreaseSpeed()
{
    flexi.IncreaseSpeed();
}
//Decrease Speed with user input
void DecreaseSpeed()
{
    flexi.DecreaseSpeed();
}
