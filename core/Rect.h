
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


#ifndef _RECT_H_
#define _RECT_H_

#include "Common.h"

class rect
{
  public:
	rect(sShort _x=0, sShort _y=0, uShort _w=0, uShort _h=0);
	rect(const rect& r);
	
	rect& operator=(const rect& r);
	
	//Returns true if x == y == w == h == 0
	bool IsDefault();
	
	bool Intersects(const rect& r);
	bool Intersects(sShort sx, sShort sy);
	
	//Returns the rect defined within an intersection of this and another rect. 
	rect Intersection(const rect& r);
	
	//Returns distance between (x, y) and (r.x, r.y). w/h are ignored during this calculation!
	double GetDistance(const rect& r);
	
	//Change the rect's x/y based on how far we're moving in a particular direction (using our 8 dir system, not abstract)
	void OffsetByDirection(direction dir, sShort distance);
	
	//Change rects x/y using polar coordinates
	void OffsetByAngle(sShort theta, sShort distance);
	
	uShort w, h;
	sShort x, y;
};

#endif //_RECT_H_
