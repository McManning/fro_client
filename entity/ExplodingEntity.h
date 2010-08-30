
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
