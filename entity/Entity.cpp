
#include "Entity.h"
#include "EntityManager.h"
#include "../map/Map.h"
#include "../core/io/Crypt.h"
#include "../core/SDL/SDL_gfxPrimitives.h"

Entity::Entity()
{
	mSolid = false;
	mVisible = true;
	mPosition.x = -1;
	mPosition.y = -1; 
	mOrigin.x = 0;
	mOrigin.y = 0;
	mSnapToGrid = false;
	mMap = NULL;
	mLayer = 0;
	mLocked = false;
	mShadow = false;
	mJumpHeight = 0;
	mCanClick = false;
}

Entity::~Entity()
{
	//destroy anything that may be trying to link to us
	timers->RemoveMatchingUserData(this);

	if (mMap)
		mMap->mBubbles.PopBubble(this);
}

string Entity::GetTypeName()
{
	switch (mType)
	{
		case ENTITY_REMOTEACTOR: return "remote";
		case ENTITY_LOCALACTOR: return "local";
		case ENTITY_SCENEACTOR: return "actor";
		case ENTITY_STATICOBJECT: return "object";
		case ENTITY_EFFECT: return "effect";
		case ENTITY_TEXT: return "text";
		default: return "unknown";
	}
}

void Entity::SetVisible(bool v)
{
	mVisible = v;	
}

bool Entity::IsVisibleInCamera() 
{ 
	return IsVisible() && mMap && mMap->IsRectInCamera(GetBoundingRect());
}

void Entity::SetPosition(point2d position)
{
	if (mSnapToGrid)
	{
		//recalculate and properly snap coordinates to our grid
		position.x /= 16;
		position.x = position.x * 16 + 8;
		position.y /= 16;
		position.y = position.y * 16;
	}
	
	//If this is the first time we call SetPosition, line up previous with current.
	if (isDefaultPoint2d(mPosition))
		mPreviousPosition = position;
	else
		mPreviousPosition = mPosition;
		
	mPosition = position;	

	if (mMap)
		mMap->QueueEntityResort();
}

bool Entity::CollidesWith(rect r)
{
	rect rr;
	for (uShort i = 0; i < mCollisionRects.size(); i++)
	{
		rr = mCollisionRects.at(i);
		rr.x += mPosition.x - mOrigin.x;
		rr.y += mPosition.y - mOrigin.y;
		
		if (areRectsIntersecting(rr, r))
			return true;
	}
	return false;
}

/*	Returns true if any of this entities rects are intersecting a solid entity */
bool Entity::IsCollidingWithSolid()
{
	ASSERT(mMap);
	rect r;
	for (uShort i = 0; i < mCollisionRects.size(); i++)
	{
		r = mCollisionRects.at(i);
		r.x += mPosition.x - mOrigin.x;
		r.y += mPosition.y - mOrigin.y;
		if (mMap->IsRectBlocked(r))
			return true;
	}
	return false;
}

bool Entity::IsCollidingWithEntity(Entity* e)
{
	rect r;
	for (uShort i = 0; i < mCollisionRects.size(); i++)
	{
		r = mCollisionRects.at(i);
		r.x += mPosition.x - mOrigin.x;
		r.y += mPosition.y - mOrigin.y;
		if (e->CollidesWith(r))
			return true;
	}
	return false;
}

rect Entity::GetBoundingRect()
{
	return rect();	
}

void Entity::RenderShadow()
{
	if (!mShadow || !mMap)
		return;
		
	/*	This SHOULD detect what sort of objects are under us, do special shadow calculations, blah blah blah.
		But, I'm lazy. So enjoy an oval.
	*/

	rect r = GetBoundingRect();
	r.x = mPosition.x;
	r.y = mPosition.y;
	r = mMap->ToScreenPosition( r );

	r.w = (r.w / 4) - 1;
	r.h = r.w / 2;
	r.y -= r.h / 2 + 1;

	filledEllipseRGBA(Screen::Instance()->Surface(), r.x, r.y,
				 	r.w, r.h, 0, 0, 0, 150);

}

void Entity::SetLayer(int l)
{
	mLayer = l;
}

void Entity::SetFlag(string flag, string value)
{
	flag = base64_encode(flag.c_str(), flag.length());
	value = base64_encode(value.c_str(), value.length());
	mFlags[flag] = value;
}

string Entity::GetFlag(string flag)
{
	flag = base64_encode(flag.c_str(), flag.length());
	string value = mFlags[flag];
	
	if (value.empty())
		return value;
	
	return base64_decode(value.c_str(), value.length());
}
