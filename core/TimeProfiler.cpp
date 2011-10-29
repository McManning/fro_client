
/*
 * Copyright (c) 2011 Chase McManning
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights 
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 */


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

