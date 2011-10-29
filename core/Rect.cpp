
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


#include "Rect.h"

rect::rect(sShort _x=0, sShort _y=0, uShort _w=0, uShort _h=0)
{
	x = _x;
	y = _y;
	w = _w;
	h = _h;
}

rect::rect(const rect& r)
{
	*this = r;
}

rect& rect::operator=(const rect& r)
{
	if (this != *r)
	{
		x = r.x;
		y = r.y;
		w = r.w;
		h = r.h;
	}
	return *this;
}

bool rect::IsDefault()
{
	return (w == 0 && h == 0 && x == 0 && y == 0);
}

bool rect::Intersects(const rect& r)
{
    if (Intersects(r.x, r.y)) return true;
    if (Intersects(r.x + r.w-1, r.y + r.h-1)) return true;
    if (Intersects(r.x, r.y + r.h-1)) return true;
    if (Intersects(r.x + r.w-1, r.y)) return true;

    if (r.Intersects(x, y)) return true;
    if (r.Intersects(x + w-1, y + h-1)) return true;
    if (r.Intersects(x, y + h-1)) return true;
    if (r.Intersects(x + w-1, y)) return true;

    return false;
}

bool rect::Intersects(sShort sx, sShort sy)
{
	return ( 	sx >= x && sx < x + w-1
				&& sy >= y && sy < y + h-1 );
}

rect rect::Intersection(const rect& r)
{
	rect result;
	
	result.x = MAX(x, r.x);
	result.y = MAX(y, r.y);
	sShort x2 = MIN(x+w, r.x+r.w);
	sShort y2 = MIN(y+h, r.y+r.h);
	
	//check for invalid intersection
	if (x2 < result.x || y2 < result.y)
		return rect();

	result.w = x2 - result.x;
	result.h = y2 - result.y;
	
	return result;
}

//Returns distance between (x, y) and (r.x, r.y). w/h are ignored during this calculation!
double rect::GetDistance(const rect& r)
{
	return sqrt( (x - r.x) * (x - r.x) + (y - r.y) * (y - r.y) );
}

//Change the rect's x/y based on how far we're moving in a particular direction (using our 8 dir system, not abstract)
void rect::OffsetByDirection(direction dir, sShort distance)
{
	//OffsetByAngle(directionToAngle(dir), distance);
	/*Can't use the angle calculation because 45deg and whatnot doesn't work
		with our current movement calculations. Yes I know diagonals are
		going to be a bit faster and further than moving horiz/vert, but the
		grid is small and not very noticeable
	*/
	switch (dir)
	{
		case SOUTHEAST:
			OffsetByAngle(90, distance); //do south
			OffsetByAngle(0, distance); //do east
			break;
		case SOUTHWEST:
			OffsetByAngle(90, distance); //do south
			OffsetByAngle(180, distance); //do west
			break;
		case NORTHEAST:
			OffsetByAngle(270, distance); //do north
			OffsetByAngle(0, distance); //do east
			break;
		case NORTHWEST:
			OffsetByAngle(270, distance); //do north
			OffsetByAngle(180, distance); //do west
			break;
		default: //angle offset is fine for horiz/vert
			OffsetByAngle(directionToAngle(dir), distance);
			break;
	}
}

//Change rects x/y using polar coordinates
void rect::OffsetByAngle(sShort theta, sShort distance)
{
	x += static_cast<sShort>(distance * cos(theta * M_PI / 180));
	y += static_cast<sShort>(distance * sin(theta * M_PI / 180));
}


