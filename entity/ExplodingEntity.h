
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


#ifndef _EXPLODINGENTITY_H_
#define _EXPLODINGENTITY_H_

#include "Entity.h"

class ExplodingEntity : public Entity
{
  public:
	ExplodingEntity(Map* map, Image* img, point2d position);
	~ExplodingEntity();

	rect GetBoundingRect();
	
	struct explodingEntityParticle
	{
		rect src;
		point2d position;
		point2d velocity;
		byte speed; //not necessary for melting image, but for testing different effects
		byte life;
	};

	void Render();
	void Think(uLong ms);

	void _build(Image* img, point2d position, rect clip);
	
	Image* mImage;
	std::vector<explodingEntityParticle> mParticles;
};

#endif //_EXPLODINGENTITY_H_
