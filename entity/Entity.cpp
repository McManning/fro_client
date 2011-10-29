
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
	mManagerCanDeleteMe = true;
	mDead = false;
	mClickRange = 0;
	mActiveChatBubble = NULL;
}

Entity::~Entity()
{
	//destroy anything that may be trying to link to us
	timers->RemoveMatchingUserData(this);
	downloader->NullMatchingUserData(this);
	
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
		case ENTITY_DAMAGEICON: return "damageicon";
		default: return "unknown";
	}
}

void Entity::SetVisible(bool v)
{	
	if (v != mVisible)
	{	
		// if we're trying to hide them, add the rect before toggling visibility
		if (mVisible && !v)
			AddPositionRectForUpdate();	
		
		mVisible = v;
		
		// write anyway, in case it went from invis->vis
		AddPositionRectForUpdate();	
	}
}

void Entity::SetShadow(bool b)
{
	mShadow = b;
	AddPositionRectForUpdate();		
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
		return mMap->IsRectInCamera(r);
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
		position.x /= 8;
		position.x = position.x * 8 + 8;
		position.y /= 8;
		position.y = position.y * 8;
	}
	
	if (position.x != mPosition.x || position.y != mPosition.y)
	{	
		if (mMap && position.y != mPosition.y)
			mMap->QueueEntityResort();
		
		//If this is the first time we call SetPosition, line up previous with current.
		if (isDefaultPoint2d(mPosition))
			mPreviousPosition = position;
		else
			mPreviousPosition = mPosition;
	
		AddPositionRectForUpdate(); // Update old position
		mPosition = position;	
		AddPositionRectForUpdate(); // Update new position
	}
}

//	Will add the bounding rect of this entity to the screen's update manager
//	Called when the entity moves, is created, deleted, changes state in any (visible) way.
void Entity::AddPositionRectForUpdate()
{
	rect r, s;
	
	if ( mMap && IsVisibleInCamera() )
	{
		r = GetBoundingRect();	
		
		if (!IsPositionRelativeToScreen())
		    r = mMap->ToScreenPosition( r );
		
		// TODO: better calculation of where our shadow actually is!
		if (mShadow)
	        r.h += r.w / 10;

		g_screen->AddRect(r);
		
		// Add the old rect too, in case of a size change or something else
	   //g_screen->AddRect(mOldPositionRect);
		//mOldPositionRect = r;
	}
}

bool Entity::CollidesWith(rect r)
{
	// Screen relative entities can't have collision info
	if (IsPositionRelativeToScreen())
		return false; 
		
	rect rr;
	for (int i = 0; i < mCollisionRects.size(); i++)
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
	for (int i = 0; i < mCollisionRects.size(); i++)
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
	for (int i = 0; i < mCollisionRects.size(); i++)
	{
		r = mCollisionRects.at(i);
		r.x += mPosition.x - mOrigin.x;
		r.y += mPosition.y - mOrigin.y;
		if (e->CollidesWith(r))
			return true;
	}
	return false;
}

void Entity::Render()
{
	if (mMap->mShowDebug) // Show debugging information 
	{
		rect r = GetBoundingRect();

		// if we're based on map position, convert
		if (!IsPositionRelativeToScreen())
			r = mMap->ToScreenPosition( r );
			
		Image* scr = Screen::Instance();
		scr->DrawRect(r, color(255,0,255), false);
		
		// X in the middle of objects. Doesn't play well when objects go offscreen
		//scr->DrawLine(r.x, r.y, r.x + r.w, r.y + r.h, color(255,0,255), 1);
		//scr->DrawLine(r.x+r.w, r.y, r.x, r.y + r.h, color(255,0,255), 1);
		
		//draw collisions
		for (int i = 0; i < mCollisionRects.size(); i++)
		{
			r = mCollisionRects.at(i);
			r.x += mPosition.x - mOrigin.x;
			r.y += mPosition.y - mOrigin.y;
			if (!IsPositionRelativeToScreen())
				r = mMap->ToScreenPosition( r );
				
			scr->DrawRect(r, color(255,0,0), false);
		}
	}
}	

void Entity::RenderShadow()
{
	if (!mShadow || !mMap)
		return;
		
	/*	This SHOULD detect what sort of objects are under us, do special shadow calculations, blah blah blah.
		But, I'm lazy. So enjoy an oval.
	*/

	rect r = GetShadowRect();
	
	// if we're based on map position, convert
	if (!IsPositionRelativeToScreen())
		r = mMap->ToScreenPosition( r );

	// PROBLEM: Can't figure out a way to do this.. where it remains within the screen rects.
	filledEllipseRGBA(g_screen->Surface(), r.x, r.y,
				 	r.w, r.h, 0, 0, 0, 150);

}

// Get the position of our shadow in rect form
// Rect is map position, not screen
rect Entity::GetShadowRect()
{
	rect r = GetBoundingRect();
	r.x = mPosition.x;
	r.y = mPosition.y;
	
	r.w = (r.w / 4) - 1;
	r.h = r.w / 2;
	r.y -= r.h / 2 + 1;
	
	return r;
}

void Entity::SetLayer(int l)
{
	mLayer = l;
	AddPositionRectForUpdate();
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

/*	index - Index of the stack where our new value for the property should be */
int Entity::LuaSetProp(lua_State* ls, string& prop, int index)
{
	if (prop == "id") mId = lua_tostring(ls, index);
	else if (prop == "name") SetName( lua_tostring(ls, index) );
	else if (prop == "visible") SetVisible( lua_toboolean(ls, index) );
	else if (prop == "solid") SetSolid( lua_toboolean(ls, index) );
	else if (prop == "shadow") SetShadow( lua_toboolean(ls, index) );
	else if (prop == "layer") SetLayer( (int)lua_tonumber(ls, index) );
	else if (prop == "clickable") mClickRange = (int)lua_tonumber(ls, index);
	else return 0;
	
	return 1;
}

int Entity::LuaGetProp(lua_State* ls, string& prop)
{
	if (prop == "id") lua_pushstring( ls, mId.c_str() );
	else if (prop == "name") lua_pushstring( ls, GetName().c_str() );
	else if (prop == "visible") lua_pushboolean( ls, IsVisible() );
	else if (prop == "solid") lua_pushboolean( ls, IsSolid() );
	else if (prop == "shadow") lua_pushboolean( ls, mShadow );
	else if (prop == "layer") lua_pushnumber( ls, GetLayer() );
	else if (prop == "ctype") lua_pushstring( ls, GetTypeName().c_str() );
	else if (prop == "clickable") lua_pushnumber( ls, mClickRange );
	else if (prop == "dead") lua_pushboolean(ls, mDead );
	else return 0;
	
	return 1;
}

void Entity::ClearActiveChatBubble()
{
	if (mActiveChatBubble && mMap)
	{
		mActiveChatBubble->mOwner = NULL;
		mMap->RemoveEntity(mActiveChatBubble);
		mActiveChatBubble = NULL;
	}
}

void Entity::Say(string msg, bool bubble, bool inChat)
{
	if (bubble)
	{
		ChatBubble* cb = new ChatBubble(this, msg);
		cb->mMap = mMap;
		mMap->AddEntity(cb);
	}

    if (inChat && mMap && mMap->mChat)
	   mMap->mChat->AddMessage(GetName() + ": " + msg);	
}

