
#ifndef _TIMEPROFILER_H_
#define _TIMEPROFILER_H_

#include "Common.h"

/*	Quick class thrown together to profile the average time it takes to perform various actions 
	The following options must be displayed:
		-The average amount of time this profiler was started/stopped in a second
		-The most amount of time ... in a second
		-The least ... in a second
		-The average amount of time lapsed between Start() and Stop()
		-... most ...
		-... least ...
*/
class TimeProfiler
{
  public:
	TimeProfiler(string id);
	~TimeProfiler();
	
	//start a profiler test
	void Start();
	
	//stop the previously started profiler test
	void Stop();

	//output profiler results
	void Print();
	
  protected:
	string mId; //identifier for this profiler

	//Time between Start() and Stop()
	uLong mMaxTime;
	uLong mMinTime;
	uLong mAverageTime;

	//keeps track of how often this profiler runs a second
	uLong mMaxRunsPerSecond;
	uLong mMinRunsPerSecond;
	uLong mAverageRunsPerSecond;

//Below variables are internal counters, not involved in the results
	
	uLong mRunsPerSecond; //counter for our runs
	uLong mStartTime; //set on Start() to keep track
	uLong mStartRuns; //tick on which we considered a second start.
};

#endif //_TIMEPROFILER_H_
