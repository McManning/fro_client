
#include <lua.hpp>
#include "Entity.h"
#include "EntityManager.h"
#include "ChatBubble.h"
#include "../map/Map.h"
#include "../core/io/Crypt.h"
#include "../core/io/FileIO.h" //getTemporaryCacheFilename
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
	mClickRange = 0;
	mActiveChatBubble = NULL;
}

Entity::~Entity()
{
	//destroy anything that may be trying to link to us
	timers->RemoveMatchingUserData(this);

	ClearActiveChatBubble();
}

string Entity::GetTypeName()
{
	switch (mType)
	{
		case ENTITY_REMOTEACTOR: return "remote";
		case ENTITY_LOCALACTOR: return "local";
		case ENTITY_ACTOR: return "actor";
		case ENTITY_STATICOBJECT: return "object";
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
	if (!IsVisible() || !mMap)
		return false;
	
	rect r = GetBoundingRect();	
	if (IsPositionRelativeToScreen())
	{
		return areRectsIntersecting(r, mMap->GetScreenPosition());
	}
	else
	{
		return mMap->IsRectInCamera(GetBoundingRect());
	}
}

bool Entity::IsPositionRelativeToScreen()
{
	return ((mLayer >= EntityManager::LAYER_STATIC_LOWER_START 
				&& mLayer <= EntityManager::LAYER_STATIC_LOWER_END)
			|| (mLayer >= EntityManager::LAYER_STATIC_UPPER_START 
				&& mLayer <= EntityManager::LAYER_STATIC_UPPER_END)
			);
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
	// Screen relative entities can't have collision info
	if (IsPositionRelativeToScreen())
		return false; 
		
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
	if (IsPositionRelativeToScreen())
		return false;
		
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
	if (IsPositionRelativeToScreen())
		return false;
		
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
	
	// if we're based on map position, convert
	if (!IsPositionRelativeToScreen())
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

int Entity::LoadCollisionFile(string file)
{
	string tmpFile = getTemporaryCacheFilename();
	if (!decompressFile(file, tmpFile))
		return 0;
		
	FILE* fin = fopen(tmpFile.c_str(), "r");

	if (!fin)
	{
		removeFile(tmpFile);
		return 0;
	}
	
	rect r;
	while (true)
	{
		if (fread((void*)&r.x, sizeof(r.x), 1, fin) != 1)
			break;
		if (fread((void*)&r.y, sizeof(r.y), 1, fin) != 1)
			break;
		if (fread((void*)&r.w, sizeof(r.w), 1, fin) != 1)
			break;
		if (fread((void*)&r.h, sizeof(r.h), 1, fin) != 1)
			break;
			
		mCollisionRects.push_back(r);
	}

	fclose(fin);
	removeFile(tmpFile);
	return 1;
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

void Entity::ClearFlag(string flag)
{
	flag = base64_encode(flag.c_str(), flag.length());
	mFlags.erase(flag);
}

/*	index - Index of the stack where our new value for the property should be */
int Entity::LuaSetProp(lua_State* ls, string& prop, int index)
{
	if (prop == "id") mId = lua_tostring(ls, index);
	else if (prop == "name") mName = lua_tostring(ls, index);
	else if (prop == "visible") SetVisible( lua_toboolean(ls, index) );
	else if (prop == "solid") SetSolid( lua_toboolean(ls, index) );
	else if (prop == "shadow") mShadow = lua_toboolean(ls, index);
	else if (prop == "layer") SetLayer( (int)lua_tonumber(ls, index) );
	else if (prop == "clickable") mClickRange = (int)lua_tonumber(ls, index);
	else return 0;
	
	return 1;
}

int Entity::LuaGetProp(lua_State* ls, string& prop)
{
	if (prop == "id") lua_pushstring( ls, mId.c_str() );
	else if (prop == "name") lua_pushstring( ls,mName.c_str() );
	else if (prop == "visible") lua_pushboolean( ls, IsVisible() );
	else if (prop == "solid") lua_pushboolean( ls, IsSolid() );
	else if (prop == "shadow") lua_pushboolean( ls, mShadow );
	else if (prop == "layer") lua_pushnumber( ls, GetLayer() );
	else if (prop == "type") lua_pushstring( ls, GetTypeName().c_str() );
	else if (prop == "clickable") lua_pushnumber( ls, mClickRange );
	else return 0;
	
	return 1;
}

void Entity::ClearActiveChatBubble()
{
	if (mActiveChatBubble && mMap)
	{
		mActiveChatBubble->mOwner = NULL;
		mMap->RemoveEntity(mActiveChatBubble);
	}
}

