
#include "Weapon.h"
#include "../entity/Actor.h"
#include "../map/Map.h"

uShort timer_SwordUpdate(timer* t, uLong ms)
{
	Sword* s = (Sword*)t->userData;
	
	//bad sword, or attack is done. 
	if (!s || s->Update())
		return TIMER_DESTROY;
		
	return TIMER_CONTINUE;
}

Sword::Sword() : Weapon()
{
	DEBUGOUT("Creating Sword");
	mImage = resman->LoadImg("assets/sword.png");
	mRenderedImage = NULL;
	mAttackTimer = NULL;
}

Sword::~Sword()
{
	resman->Unload(mImage);
	resman->Unload(mRenderedImage);
	timers->Remove(mAttackTimer);
}

void Sword::RenderUnder()
{
	if (mState == IDLE || !mRenderedImage)
		return;

	if (mDegree > 180) //"higher" than player, draw under
		_renderBlade();
}

void Sword::RenderOver()
{
	if (mState == IDLE || !mRenderedImage)
		return;
		
	if (mDegree <= 180) //"lower" than player, draw over
		_renderBlade();
}

void Sword::_renderBlade()
{
	ASSERT(mOwner);
	
	Image* scr = Screen::Instance();
		
	rect r( mOwner->mPosition.x, mOwner->mPosition.y, 0, 0 );
	
	r = mOwner->mMap->ToScreenPosition(r);
	r.y -= 16;

	offsetRectByAngle(&r, mDegree, mDistance);

	//By centering around x/y, we don't get AS MUCH jerky rotation caused by the new surfs rect
	mRenderedImage->Render(scr, r.x - (mRenderedImage->Width() / 2), r.y - (mRenderedImage->Height() / 2));
	gui->mFont->Render(scr, r.x, r.y, its(mDegree) + "deg", color(200,0,0));
}

void Sword::Use()
{
	DEBUGOUT("SWINGING SWORD");
	
	if (mState == ATTACKING)
		return;
	
	ASSERT(mOwner);
	
	mState = ATTACKING;
	
	mDegree = 0;
	mSpeed = 36;
	mDistance = 40;
	
	//set our swing range
	switch (mOwner->GetDirection())
	{
		case NORTH:
		case NORTHEAST:
		case NORTHWEST:
			mStartDegree = 315;
			mEndDegree = 225;
			break;
		case SOUTH:
		case SOUTHEAST:
		case SOUTHWEST:
			mStartDegree = 135;
			mEndDegree = 45;
			break;
		case EAST:
			mStartDegree = 45;
			mEndDegree = 315;
			break;
		case WEST:
			mStartDegree = 225;
			mEndDegree = 135;
			break;
		default: break;	
	}
	
	mDegree = mStartDegree;
	
	timers->Add("sword", 50, false, timer_SwordUpdate, NULL, this);
}

bool Sword::Update()
{
	DEBUGOUT("UPDATING SWORD");
	if (mRenderedImage)
		resman->Unload(mRenderedImage);
	
	mRenderedImage = mImage->Clone(true);
	mRenderedImage->Rotate(360 - (mDegree - mSpeed), 1, 1);

	mDegree -= mSpeed;

	//Finish();
	//	return true;
		
	if (mDegree < 0)
	{
		Finish();
		return true;	
	}
//		mDegree -= 360;

	return false; //keep going
}

void Sword::Finish()
{
	DEBUGOUT("Sword Done");
	mState = IDLE;
	mAttackTimer = NULL;
}

