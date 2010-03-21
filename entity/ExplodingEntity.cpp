
#include "ExplodingEntity.h"
#include "../map/Map.h"

uShort timer_explodeImage(timer* t, uLong ms)
{
	ExplodingEntity* m = (ExplodingEntity*)t->userData;

	if (m)
	{
		m->Think(ms);

		//if we don't have any particles left, destroy the emitter
		return (m->mParticles.empty()) ? TIMER_DESTROY : TIMER_CONTINUE;
	}
	else
	{
		return TIMER_DESTROY;
	}

}

void timer_eraseExplodingEntity(timer* t)
{
	ExplodingEntity* m = (ExplodingEntity*)t->userData;

	if (m)
	{
		//if it's not found on the map (map changed before this entity died) delete it here instead to avoid a leak.
		if (!m->mMap->RemoveEntity(m, ENTITYLEVEL_SKY))
			delete m;
	}
}

ExplodingEntity::ExplodingEntity(Map* map, Image* img, point2d position)
{
	ASSERT(img);
	ASSERT(map);
	
	mType = ENTITY_EFFECT;
	SetSolid(false);
	
	mImage = NULL;
	mId = "ExplodingEntity";
	mMap = map;

	mTimer = timers->Add("boom", 10, false,
						timer_explodeImage,
						timer_eraseExplodingEntity,
						this);
						
	mMap->AddEntity(this, ENTITYLEVEL_SKY);

	_build(img, position, rect(0,0,img->Width(),img->Height()));
}

ExplodingEntity::~ExplodingEntity()
{
	mTimer->userData = NULL; //so the erase callback won't try to delete us
	timers->Remove(mTimer);

	mParticles.clear();
		
	resman->Unload(mImage);

}

rect ExplodingEntity::GetBoundingRect()
{
	return rect(mPosition.x, mPosition.y, mImage->Width(), mImage->Height());	
}

//Recalculate positions of every particle
void ExplodingEntity::Think(uLong ms)
{
	for (int i = 0; i < mParticles.size(); i++)
	{
		explodingEntityParticle& p = mParticles.at(i);

		p.position.y += p.velocity.y * p.speed;
		p.position.x += p.velocity.x * p.speed;

		p.life--;
		if (p.life < 1) //this particle must die now
		{
			mParticles.erase(mParticles.begin() + i);
			i--;
		}
	}
}

//draw all particle clips at their specified locations
void ExplodingEntity::Render(uLong ms)
{
	if (!mImage) return;

	Image* scr = Screen::Instance();
	
	rect r;
	for (int i = 0; i < mParticles.size(); i++)
	{
		explodingEntityParticle& p = mParticles.at(i);

		r.x = p.position.x;
		r.y = p.position.y;

		r = mMap->ToScreenPosition(r);

		mImage->Render(scr, r.x, r.y, p.src);
	}
}

void ExplodingEntity::_build(Image* img, point2d position, rect clip)
{
	mImage = img->Clone();
	
	ASSERT(mImage);
	mImage->Stop(); //If it's exploding, it wouldn't need to be animated.
	
	SetPosition(position);
	
	uShort particleSize = 2;

	explodingEntityParticle p;

	for (int x = 0; x < clip.w; x += particleSize)
	{
		for (int y = 0; y < clip.h; y += particleSize)
		{
			p.position.x = mPosition.x + x;
			p.position.y = mPosition.y + y;
			p.speed = rnd2(1, 3);
			p.life = rnd2(20, 50);

			do //can't get 0 velocity
			{
				p.velocity.x = rnd2(-2, 2);
				p.velocity.y = rnd2(-2, 2);
			}
			while (p.velocity.x == 0 && p.velocity.y == 0);

			//adjust oversized particles into rect
			p.src.w = p.src.h = particleSize;
			if (x + p.src.w > clip.w)
				p.src.w -= x + p.src.w - clip.w;
			if (y + p.src.h > clip.h)
				p.src.h -= y + p.src.h - clip.h;

			p.src.x = x + clip.x;
			p.src.y = y + clip.y;

			mParticles.push_back(p);
		}
	}
}
