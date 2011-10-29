
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
