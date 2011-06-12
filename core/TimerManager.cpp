
#include "TimerManager.h"
#include <SDL/SDL.h>

TimerManager* timers;

TimerManager::TimerManager()
{
	mCurrentHandle = 1; //handle 0 is invalid.
	mResortTimers = false;
}

TimerManager::~TimerManager()
{
	FlushTimers();
}

void TimerManager::FlushTimers()
{
	timer* t;
	for (int i = 0; i < mTimers.size(); i++)
	{
		t = mTimers.at(i);
		if (t)
		{
			PRINT("Flush: " + t->id + " " + pts(t) + " UserData: " + pts(t->userData));
			if (t->onDestroy)
				t->onDestroy(t);

			SAFEDELETE(t);
		}
	}
	mTimers.clear();
}

bool timerSort( timer* a, timer* b )
{
	return ( (a->interval < b->interval) );
}

void TimerManager::Process(uLong ms)
{

	for (int i = 0; i < mTimers.size(); i++)
	{
		if ( !mTimers.at(i) ) //Erase any nulled timers
		{
			mTimers.erase(mTimers.begin() + i);
			i--;
		}
		else if ( _doTimer(ms, mTimers.at(i)) == TIMER_DESTROY )
		{
			_delete(i);
			mTimers.erase(mTimers.begin() + i);
			i--;
		}
	}
	
/*	if (mResortTimers)
	{
		mResortTimers = false;
		stable_sort(mTimers.begin(), mTimers.end(), timerSort);
	}*/
}

timer* TimerManager::Add(string id, uLong interval, bool runImmediately,
			uShort (*onActivate)(timer*, uLong),
			void (*onDestroy)(timer*),
			void* userData)
{

	timer* t = new timer();
	t->id = id;
	t->interval = interval;
	t->onActivate = onActivate;
	t->onDestroy = onDestroy;
	t->userData = userData;
	t->lastMs = (runImmediately) ? (SDL_GetTicks() - interval) : 0;
	t->runCount = 0;
	t->handle = ++mCurrentHandle;

	mTimers.push_back(t);
	mResortTimers = true;
	return t;
}

timer* TimerManager::AddProcess(string id, 
							uShort (*onActivate)(timer*, uLong), 
							void (*onDestroy)(timer*), 
							void* userData)
{
	timer* t = new timer();
	t->id = id;
	t->interval = PROCESSOR_INTERVAL;
	t->onActivate = onActivate;
	t->onDestroy = onDestroy;
	t->userData = userData;
	t->lastMs = 0;
	t->runCount = 0;
	t->handle = ++mCurrentHandle;

	mTimers.push_back(t);
	mResortTimers = true;
	return t;
}	

bool TimerManager::Remove(string id, void* userData)
{
	bool found = false;

	for (int i = 0; i < mTimers.size(); i++)
	{
		if ( mTimers.at(i) != NULL &&
			mTimers.at(i)->id == id )
		{
			//Check for userdata match if it's included in the search
			if (userData == NULL || mTimers.at(i)->userData == userData)
			{
				_delete(i);
				found = true;
			}
		}
	}

	return found;
}

bool TimerManager::Remove(timer* t)
{
	for (int i = 0; i < mTimers.size(); i++)
	{
		if ( mTimers.at(i) == t ) {
			_delete(i);
			return true;
		}
	}

	return false;
}

bool TimerManager::RemoveByHandle(uLong handle)
{
	if (handle != INVALID_TIMER_HANDLE)
	{
		for (int i = 0; i < mTimers.size(); i++)
		{
			if ( mTimers.at(i) && mTimers.at(i)->handle == handle ) 
			{
				_delete(i);
				return true;
			}
		}
	}

	return false;
}

bool TimerManager::RemoveMatchingUserData(void* userData)
{
    bool found = false;
	for (int i = 0; i < mTimers.size(); i++)
	{
		if ( mTimers.at(i) &&
			mTimers.at(i)->userData == userData )
		{
			_delete(i);
			found = true;
		}
	}
	return found;
}

uShort TimerManager::_doTimer(uLong ms, timer* t)
{
	ASSERT(t);

	if (t->interval == PROCESSOR_INTERVAL)
	{
		t->runCount++;
		t->lastMs = ms;
		if (t->onActivate)
			return t->onActivate(t, t->lastMs);

		return TIMER_DESTROY;
	}
	
	//make sure the timer was never given an invalid interval
	if (t->interval < 1)
		t->interval = 1;
	
	if (t->lastMs == 0) //If this timer was just created, make sure it waits the interval time before activation
	{
		t->lastMs = ms;
	}
	else if (ms > t->lastMs + t->interval)
	{
		int counter = 0;
		int lastInterval = t->interval;
		while (ms >= t->lastMs + t->interval)
		{
			//PRINT("Timer " + t->id + " (Interval: " + its(t->interval) + ") Running. Count: " + its(counter));
			counter++;
			t->lastMs += t->interval;
			t->runCount++;
			//t->lastMs = ms;

			if (!t->onActivate)
				return TIMER_DESTROY;
			else if (t->onActivate(t, t->lastMs) == TIMER_DESTROY)
				return TIMER_DESTROY;
				
			//if the timer had it's interval changed during onActivate, make sure it's still valid
			if (t->interval < 1)
			{
				t->interval = 1;
				return TIMER_CONTINUE;
			}
		}
		
		// timer changed itself, trigger a resort. 
		if (t->interval != lastInterval)
			mResortTimers = true;
	}

	return TIMER_CONTINUE;
}

void TimerManager::_delete(uShort index)
{
	if (!mTimers.at(index)) return;

	if (mTimers.at(index)->onDestroy)
		mTimers.at(index)->onDestroy( mTimers.at(index) );

	SAFEDELETE(mTimers.at(index));
	
	mResortTimers = true;
}

timer* TimerManager::Find(string id)
{
	for (int i = 0; i < mTimers.size(); i++)
	{
		if ( mTimers.at(i) != NULL &&
			mTimers.at(i)->id == id )
		{
			return mTimers.at(i);
		}
	}
	return NULL;
}

timer* TimerManager::FindByHandle(uLong handle)
{
	if (handle != INVALID_TIMER_HANDLE)
	{
		for (int i = 0; i < mTimers.size(); i++)
		{
			if ( mTimers.at(i) != NULL &&
				mTimers.at(i)->handle == handle )
			{
				return mTimers.at(i);
			}
		}
	}
	return NULL;
}

timer* TimerManager::Find(string id, void* userData)
{
	for (int i = 0; i < mTimers.size(); i++)
	{
		if ( mTimers.at(i) != NULL &&
			mTimers.at(i)->id == id && 
			mTimers.at(i)->userData == userData)
		{
			return mTimers.at(i);
		}
	}
	return NULL;
}

void TimerManager::StateToString(string& s)
{
	//print timers
	for (int i = 0; i < mTimers.size(); i++)
	{
		if (mTimers.at(i))
		{
		s += mTimers.at(i)->id
			+ "\\n    Interval: " + its(mTimers.at(i)->interval)
			+ "\\n    RunCount: " + its(mTimers.at(i)->runCount)
			+ "\\n    userData: " + pts(mTimers.at(i)->userData)
			+ "\\n    handle: " + its(mTimers.at(i)->handle);
		}
		else
		{
			s += "NULL";
		}
		s += "\\n";
	}
}

