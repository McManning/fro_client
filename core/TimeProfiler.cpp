
#include <fstream>
#include <SDL/SDL.h>
#include "TimeProfiler.h"

TimeProfiler::TimeProfiler(string id)
{
	mId = id;

	mMaxTime = 0;
	mMinTime = ULONG_MAX;
	mAverageTime = 0;
	
	mMaxRunsPerSecond = 0;
	mMinRunsPerSecond = ULONG_MAX;
	mAverageRunsPerSecond = 0;
	
	mStartTime = 0;
	mStartRuns = 0;

}

TimeProfiler::~TimeProfiler()
{
	Print();
}

void TimeProfiler::Start()
{
	mStartTime = SDL_GetTicks();
	
	if (mStartTime - mStartRuns > 1000)
	{
		mStartRuns = mStartTime;
		
		if (mRunsPerSecond > mMaxRunsPerSecond)
			mMaxRunsPerSecond = mRunsPerSecond;
		
		if (mRunsPerSecond < mMinRunsPerSecond)
			mMinRunsPerSecond = mRunsPerSecond;
			
		if (mAverageRunsPerSecond == 0)
			mAverageRunsPerSecond = mRunsPerSecond;
		else
			mAverageRunsPerSecond = (mRunsPerSecond + mAverageRunsPerSecond) / 2;
		
		mRunsPerSecond = 1;
	}
	else
	{
		++mRunsPerSecond;
	}
	
	//set it again, in case the previous action took more time than it should've
	mStartTime = SDL_GetTicks(); 
}

void TimeProfiler::Stop()
{
	uLong ms = SDL_GetTicks() - mStartTime;
	
	if (ms > mMaxTime)
		mMaxTime = ms;
	
	if (ms < mMinTime)
		mMinTime = ms;
		
	if (mAverageTime == 0)
		mAverageTime = ms;
	else
		mAverageTime = (ms + mAverageTime) / 2;
}

void TimeProfiler::Print()
{
	std::fstream f;
	f.open("logs/time_profile.log", std::fstream::out | std::fstream::app);
	
	if (f.fail())
		FATAL("time_profile write error");
		
	f << "-- Profile ID: " << mId << " [" << timestamp(true) << "] --\n";
	f << "\tMaximum Time: " << mMaxTime << " ms\n";
	f << "\tMinimum Time: " << mMinTime << " ms\n";
	f << "\tAverage Time: " << mAverageTime << " ms\n";
	f << "\tMaximum Runs/Sec: " << mMaxRunsPerSecond << "\n";
	f << "\tMinimum Runs/Sec: " << mMinRunsPerSecond << "\n";
	f << "\tAverage Runs/Sec: " << mAverageRunsPerSecond << "\n";
	f << "-- End Profile --\n\n";
	
	f.close();
}

