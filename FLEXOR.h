// FLEXOR.h

#ifndef _FLEXOR_h
#define _FLEXOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
	#include "ezButton.h"
#else
	#include "WProgram.h"
#endif

class FLEXORClass
{
 protected:
	 
	 short int min_delay_speed = 1000; //Fastest speed safely allowed

 public:
	FLEXORClass(int pulse, int dir, int ena, int _Zero, int _End, int _power1, int _power2);
	void Init(void);
	void Loop(void);
	void Enable(bool);
	void IncreaseSpeed(void);
	void DecreaseSpeed(void);
	void SetSpeed(int speed);
	void SetMinSpeed(int min);
	void IncreaseTravelDistance(void);
	void DecreaseTravelDistance(void);
	bool IsEnabled(void);
	bool IsReady(void);
	bool IsRunning(void);
	bool IsRunning(bool);
	void StartHoming(void);

	short int currentStep = 700;
private:
	int _Pulse; //Pin being used to send pulse to stepper driver
	int _Dir;//Pin being used to control direction of stepper driver
	int _ENA; //pin being used to enable/disable stepper driver
	int _Power1;
	int _Power2;

	bool enabled = false; //Toggles Main Sequence
	bool isReady = false;
	bool running = false;
	bool homing = false;
	bool recovering = false;

	bool collisionState = false;
	bool lastCollisionState = false;

	bool currentDirFWD = true; //FWD false is away from motor
	bool jerkReductionEnabled = true;

	//Speed information
	short int damping_starting_travel_ratio = 10; //This will start damping @ 1/10 the max travel at each end..  
	short int delay_speed_damping_rate = 25; //Amount changed per step while approaching end point
	short int delay_speed_change_amount = 100; //Amount changed per user input button hit

	short int max_delay_speed = 10000; //Slowest speed while damping
	short int target_delay_speed = 1000; //Desired delay speed to reach

	//used to track current delay interval
	long previousMicros = 0;
	short int delay_Micros = 1000; // current delay speed;

	 // Toggles Homing sequence
	bool ZeroHit = false; //Used for detecting when the zero point was hit to initiate step 2 of Homing sequence
	bool EndHit = false;
	short int ZeroStep = 0;
	short int EndStep = 1800;


	short int maxStep = 1800;
	 //Used to determine platform location
	 //End of the track in steps from the zero point
	short int travel = 500; // Max travel distance to move (-250, +250) from center

	short int minTravel, maxTravel, minDamperStep, maxDamperStep; //Calculated travel distance from center
	short int travel_change_amount = 50; //Amount of travel is inc/dec with user button input

	short int homeStep; //The center of the track, usually maxStep/2, but I want to add second Stop point to get an exact step count to find center in case things move around.

	short int endStopBufferSteps = 50;

	ezButton ZERO_SWITCH;
	ezButton END_SWITCH;

	void UpdateTravelParameter(void);
	void Adjust_Motor_Speed(void);
	void one_rot(void);
	void SetDirection(bool _dir);
	void loop(void);
	bool DelayFinished(long _micros);
	void CheckForDirectionChange(void);
	bool IsColliding(void);
	void RespondToCollision(void);
	
	void ZeroReached(void);
	void EndReached(void);
	void EnableJerkReduction(bool);
	void HomeReached(void);
	void Recover(void);
	void Recovered(void);
};

extern FLEXORClass FLEXOR;

#endif

