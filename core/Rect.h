
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
