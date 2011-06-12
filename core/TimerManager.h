
#ifndef _TIMERMANAGER_H_
#define _TIMERMANAGER_H_

#include "Common.h"

/*	Interval to identify processor-type timers */
#define PROCESSOR_INTERVAL (0) /*(ULONG_MAX - 1)*/
#define INVALID_TIMER_HANDLE (0)

//Values returned by a timers onActivate
enum 
{
	TIMER_CONTINUE = 0,
	TIMER_DESTROY
};

struct timer 
{
	uShort (*onActivate)(timer*, uLong);
	void (*onDestroy)(timer*);
	uLong interval; 
	void* userData;
	string id;
	uLong lastMs;
	uLong runCount; //how many times this timer went off.. good for changing events
	uLong handle; //unique identifier for this timer
};

//Manage and process timed events 
class TimerManager 
{
  public:
  
	TimerManager();
	~TimerManager();
	
	void Process(uLong ms);
	
	void FlushTimers();
	
	/*	if runImmediately is true, the timer will activate during the next 
		available heartbeat, else it'll wait interval time. 
	*/
	timer* Add(string id, uLong interval, bool runImmediately,
				uShort (*onActivate)(timer*, uLong), 
				void (*onDestroy)(timer*) = NULL, 
				void* userData = NULL);
	
		
	/*	Add's Process type timer. These will be called every time timer->Process() is called.
		Note: timer::runCount is not used for these timers. And timer::lastMs will be 0 on the first run.
	*/
	timer* AddProcess(string id, 
						uShort (*onActivate)(timer*, uLong), 
						void (*onDestroy)(timer*) = NULL, 
						void* userData = NULL);
		
	bool Remove(string id, void* userData = NULL); //Remove by ID, userdata optional
	bool Remove(timer* t);
	bool RemoveByHandle(uLong handle); //Remove the timer with the matching handle identifier
	bool RemoveMatchingUserData(void* userData); //Remove all with matching userdata
	
	timer* Find(string id);
	timer* Find(string id, void* userData);
	
	timer* FindByHandle(uLong handle);
	
	void StateToString(string& s);
	
	std::vector<timer*> mTimers;
	
	uLong mCurrentHandle; //Continuously increment for each new timer
  private:
  
	uShort _doTimer(uLong ms, timer* t);
	void _delete(uShort index);
	bool mResortTimers;
};

extern TimerManager* timers;

#endif //_TIMERMANAGER_H_
