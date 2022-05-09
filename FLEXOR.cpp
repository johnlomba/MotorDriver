// 
// 
// 

#include "FLEXOR.h"
#include "Arduino.h"
#include "ezButton.h"

FLEXORClass::FLEXORClass(int _pulse, int _dir, int _ena, int _zero, int _end, int _p1, int _p2) : ZERO_SWITCH(_zero), END_SWITCH(_end)
{
    _Pulse = _pulse;

    _Dir = _dir;

    _ENA = _ena;

    _Power1 = _p1;

    _Power2 = _p2;
}

void FLEXORClass::Init(void) 
{
    ZERO_SWITCH.setDebounceTime(50);
    END_SWITCH.setDebounceTime(50);

    //Voltage for Input Switches
    pinMode(_Power1, OUTPUT);
    digitalWrite(_Power1, HIGH);

    pinMode(_Power2, OUTPUT);
    digitalWrite(_Power2, HIGH);

    pinMode(_Pulse, OUTPUT);

    pinMode(_Dir, OUTPUT);

    pinMode(_ENA, OUTPUT);

    //Setup driver
    digitalWrite(_ENA, HIGH); //Turn off Driver

    //Open Serial Connection for debug output
    Serial.begin(9600);
    Serial.println("Hi, I'm flexi");
    delay(10);
}

void FLEXORClass::Loop(void) 
{
    ZERO_SWITCH.loop();
    END_SWITCH.loop();
    loop();
}

void FLEXORClass::Enable(bool _ena) 
{
    enabled = _ena;
    if (_ena) {
        digitalWrite(_ENA, LOW);
    }
    else {
        digitalWrite(_ENA, HIGH);
    }
}

//Increase Speed with user input
void FLEXORClass::IncreaseSpeed(void)
{
    target_delay_speed = constrain((target_delay_speed + delay_speed_change_amount), min_delay_speed, max_delay_speed);
}

//Decrease Speed with user input
void FLEXORClass::DecreaseSpeed(void)
{
    target_delay_speed = constrain((target_delay_speed - delay_speed_change_amount), min_delay_speed, max_delay_speed);
}

void FLEXORClass::SetSpeed(int _speed)
{
    delay_Micros = constrain(_speed, min_delay_speed, max_delay_speed);
}

void FLEXORClass::SetMinSpeed(int _speed)
{
    max_delay_speed = constrain(_speed, min_delay_speed, 10000);
}

//Increase travel distance with user input
void FLEXORClass::IncreaseTravelDistance(void)
{
    travel = travel + travel_change_amount;
    UpdateTravelParameter();
}

//Decrease travel distance with user input
void FLEXORClass::DecreaseTravelDistance(void)
{
    travel = travel - travel_change_amount;
    UpdateTravelParameter();
}

//Update the Travel min/max with new travel value
void FLEXORClass::UpdateTravelParameter(void)
{
    minTravel = homeStep - int(travel / 2);
    minTravel = constrain(minTravel, 0, maxStep);
    maxTravel = homeStep + int(travel / 2);
    maxTravel = constrain(maxTravel, 0, maxStep);

    maxDamperStep = maxTravel - int(maxTravel / damping_starting_travel_ratio);

    minDamperStep = minTravel + int(maxTravel / damping_starting_travel_ratio);

    Serial.print("MAX TRAVEL: ");
    Serial.print(maxTravel);

    Serial.print(", MIN TRAVEL: ");
    Serial.println(minTravel);
}

//Reduce Speed near edges
void FLEXORClass::Adjust_Motor_Speed(void)
{
    // Increase delay when approaching end
    if (currentDirFWD && currentStep > maxDamperStep)
    {
        delay_Micros = delay_Micros + delay_speed_damping_rate;
        delay_Micros = constrain(delay_Micros, target_delay_speed, max_delay_speed);
    }
	// Increase delay when approaching origin
    else if (!currentDirFWD && currentStep < minDamperStep)
    {
        delay_Micros = delay_Micros + delay_speed_damping_rate;
        delay_Micros = constrain(delay_Micros, target_delay_speed, max_delay_speed);
    }
	// Accelerate after changing direction
    else if (delay_Micros > target_delay_speed)
    {       
        delay_Micros = delay_Micros - delay_speed_damping_rate;
        delay_Micros = constrain(delay_Micros, target_delay_speed, max_delay_speed);
    }
}

void FLEXORClass::SetDirection(bool _dir) 
{
    currentDirFWD = _dir;
    //Set Direction of motor && Update stepper counter
    if (currentDirFWD)
    {
        Serial.println("direction set towards motor ");
        digitalWrite(_Dir, HIGH);
    }
    else
    {
        Serial.println("direction set away from motor ");
        digitalWrite(_Dir, LOW);
    }
}

bool FLEXORClass::IsRunning(void) 
{
    return running;
}

bool FLEXORClass::IsRunning(bool _run)
{
    if (_run) 
    {
        if (isReady)
        {
            running = _run;
        }
    }
    else {
        running = _run;
    }    
}

bool FLEXORClass::IsEnabled(void) 
{
    return enabled;
}

bool FLEXORClass::IsReady(void)
{
    return isReady;
}

//Send a pulse to the stepper driver
void FLEXORClass::one_rot(void)
{    
    if (jerkReductionEnabled)
    {
        Adjust_Motor_Speed();
    }

    //Send pulse
    digitalWrite(_Pulse, HIGH);
    delayMicroseconds(500); //This delay is required to make square wave pulse
    digitalWrite(_Pulse, LOW);

    if (currentDirFWD)
    {
        currentStep++;
    }
    else
    {
        currentStep--;
    }
}

void FLEXORClass::loop(void) 
{
    if (IsColliding()) 
    {
        RespondToCollision();
    }

    //Normal Sequence
    if (recovering) 
    {
        if (DelayFinished(micros()))
        {
            one_rot();
        }

        if (ZeroHit && EndHit && currentStep == homeStep)
        {
            Serial.print("Reached home = ");
            Serial.println(currentStep);
            Recovered();
        }
    }
    if (running)
    {
        if (DelayFinished(micros()))
        {
            one_rot();
            CheckForDirectionChange();
        }
    }
    else if (homing) 
    {
        if (DelayFinished(micros()))
        {
            one_rot();
        }

        if (ZeroHit && EndHit && currentStep == homeStep)
        {
            Serial.print("Reached home = ");
            Serial.println(currentStep);
            HomeReached();
        }
    }
}

void FLEXORClass::RespondToCollision(void) 
{
    //Reverse Direction on hit
    if (ZERO_SWITCH.getState() == LOW)
    {
        Serial.println("Is colliding with Zero");
        SetDirection(true); //towards motor
        currentStep = -endStopBufferSteps;

        if (!ZeroHit && homing)
        {
            Serial.print("Reached Zero in homing = ");
            Serial.println(currentStep);
            ZeroReached();
        }
        else
        {
            if (running) {
                recovering = true;
                Recover();
            }
        }
    }
    else if (END_SWITCH.getState() == LOW)
    {
        Serial.println("Is colliding with End");
        SetDirection(false); //Away from motor

        if (!EndHit && ZeroHit && homing)
        {
            Serial.print("Reached End in homing = ");
            Serial.println(currentStep);
            EndReached();
        }
        else 
        {
            if (running) {
                recovering = true;
                Recover();
            }          
        }
    }

    //Stop Device if both switches are being hit
    if (END_SWITCH.getState() == LOW && ZERO_SWITCH.getState() == LOW) 
    {
        Enable(false);
        IsRunning(false);
        homing = false;
    }
}

void FLEXORClass::HomeReached(void)
{
    isReady = true;
    homing = false;
    SetSpeed(target_delay_speed);
    UpdateTravelParameter();
    EnableJerkReduction(true);
    IsRunning(false);
}

void FLEXORClass::ZeroReached(void) 
{
    ZeroHit = true;
}

void FLEXORClass::EndReached(void) 
{
    if (ZeroHit)
    {
        maxStep = currentStep - endStopBufferSteps;
        homeStep = int(currentStep / 2);
        EndHit = true;
    }
}

bool FLEXORClass::IsColliding(void) 
{
    if (ZERO_SWITCH.getState() == 0 || END_SWITCH.getState() == 0) 
    {
        Serial.println("Is colliding");
        return true;
    }

    return false;
}

void FLEXORClass::StartHoming(void) 
{
    //Start homing going towards motor  _start_dir == true
    EnableJerkReduction(false); //Going slow the whole time
    IsRunning(false); //Turn off the main sequence
    Enable(true);     //Turn on motor
    SetSpeed(5000);  //Set speed slow
    SetDirection(false);
    homing = true; //Turn on the homing sequence in the loop
    ZeroHit = false; //Reset Zero point switch trigger
    EndHit = false;  //Waiting until I add switch to reet trigger

    //Bypass Travel to allow for maximum range
    maxTravel = maxStep;
    minTravel = 0;
}

void FLEXORClass::Recover(void)
{
    //Start homing going towards motor  _start_dir == true
    EnableJerkReduction(false); //Going slow the whole time
    IsRunning(false); //Turn off the main sequence
    Enable(true);     //Turn on motor
    SetSpeed(5000);  //Set speed slow
    SetDirection(false);
    homing = true; //Turn on the homing sequence in the loop
    ZeroHit = false; //Reset Zero point switch trigger
    EndHit = false;  //Waiting until I add switch to reet trigger

    //Bypass Travel to allow for maximum range
    maxTravel = maxStep;
    minTravel = 0;
}

void FLEXORClass::Recovered(void)
{
    isReady = true;
    homing = false;
    recovering = false;
    SetSpeed(target_delay_speed);
    UpdateTravelParameter();
    EnableJerkReduction(true);
    IsRunning(true);
}

void FLEXORClass::EnableJerkReduction(bool _ena) 
{
    jerkReductionEnabled = _ena;
}

void FLEXORClass::CheckForDirectionChange(void) 
{

    if (currentStep >= maxTravel)
    {
        SetDirection(false);
    }
    else if (currentStep <= minTravel)
    {
        SetDirection(true);
    }
}

//Check to see if enough time has gone by before we can pulse the motor again (this controls speed by delaying pulse)
bool FLEXORClass::DelayFinished(long _micros)
{
    if (_micros - previousMicros >= delay_Micros)
    {
        previousMicros = _micros;
        return true;
    }

    return false;
}

