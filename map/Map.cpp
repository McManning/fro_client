
#include "Map.h"
#include "../entity/RemoteActor.h"
#include "../core/widgets/Input.h"
#include "../core/widgets/OpenUrl.h"
#include "../core/io/Crypt.h"
#include "../game/GameManager.h"
#include "../lua/MapLib.h"
#include "../interface/AvatarFavoritesDialog.h"
#include "../interface/RemotePlayerMenu.h"

Map::Map() 
	: Frame(NULL, "", rect())
{ 
	mWidth = mHeight = 0; 
	mType = NONE;
	mFollowedEntity = NULL;
	mStopCameraAtMapEdge = true;
	mLuaState = NULL;
	mBubbles.mMap = this;
	mShowPlayerNames = game->mPlayerData.GetParamInt("map", "shownames");
	mGravity = 1;
	mCameraSpeed = 4;
}

Map::~Map()
{
	SaveFlags();
	
	//in case this wasn't deleted gracefully
	if (mLuaState)
		mapLib_CloseLuaState(mLuaState);
}

void Map::Render(uLong ms)
{
	mBubbles.Render(ms);
	Frame::Render(ms);
}

void Map::Event(SDL_Event* event)
{
	Entity* e;
	switch (event->type)
	{
		case SDL_MOUSEBUTTONUP:
			if (event->button.button == SDL_BUTTON_RIGHT)
			{
				HandleRightClick();
			}
			else if (event->button.button == SDL_BUTTON_LEFT)
			{
				HandleLeftClick();	
			}
			break;
		case SDL_KEYDOWN: {
			MessageData md("KEY_DOWN");
			md.WriteInt("key", event->key.keysym.sym);
			messenger.Dispatch(md, NULL);
		} break;
		case SDL_KEYUP: {
			MessageData md("KEY_UP");
			md.WriteInt("key", event->key.keysym.sym);
			messenger.Dispatch(md, NULL);
		
		} break;
		default: break;	
	}
	
	//make sure keys go to our input box
	if (game && HasKeyFocus())
		game->mChat->mInput->SetKeyFocus();	
}

void Map::HandleLeftClick()
{
	Entity* e = GetEntityUnderMouse(true, false);
	
	if (e)
	{
		MessageData md("ENTITY_CLICK");
		md.WriteUserdata("entity", e);
		messenger.Dispatch(md, e);
	}
}

void Map::HandleRightClick()
{
	Entity* e = GetEntityUnderMouse(false, true);
	
	if (e == (Entity*)game->mPlayer)
	{
		if (!gui->Get("avyfavs"))
			new AvatarFavorites();
	}
	else if (e) //remote player
	{
		ClickRemoteActor((RemoteActor*)e);
	}
}

void Map::ClickRemoteActor(RemoteActor* ra)
{
	//Console* c = game->GetPrivateChat(ra->mName);
	new RemotePlayerMenu(this, ra);
}

/*	Will attempt to return the entity directly under the mouse, if it is clickable. */
Entity* Map::GetEntityUnderMouse(bool mustBeClickable, bool playersOnly)
{
	int x, y;
	rect r;
	Entity* e;
	Image* img;
	
	//for all entities, highest to lowest
	for (int i = mEntities.size()-1; i > -1; --i)
	{
		e = mEntities.at(i);
			
		//if we can click this entity, (clickable OR playersOnly mode and it's considered a player)
		if (!e || !e->IsVisibleInCamera())
			continue;
	
		if (mustBeClickable && !e->mCanClick)
			continue;
			
		if (playersOnly && e->mType != ENTITY_LOCALACTOR && e->mType != ENTITY_REMOTEACTOR)
			continue;

		r = ToScreenPosition(e->GetBoundingRect());
		if ( areRectsIntersecting(gui->GetMouseRect(), r) )
		{
			//Otherwise, if the mouse is touching a non transparent pixel
			x = gui->GetMouseX() - r.x;
			y = gui->GetMouseY() - r.y;
			
			img = e->GetImage();
			if (img)
			{
				//if non transparent pixel, we found our entity. 
				if (!img->IsPixelTransparent(x, y))
					return e;
			}
		}
	}
	
	return NULL;
}

void Map::Process(uLong ms)
{
	mBubbles.Process(ms);
	UpdateCamera(ms);
	ResortEntities();
}

/*	A graceful cleanup was issued. Do some special cleanup */
void Map::Die()
{
	//tell our primary script we're going to die, before we do.
	mapLib_CloseLuaState(mLuaState);
	mLuaState = NULL;
	
	FlushEntities();
	
	Widget::Die();
}

bool Map::IsRectBlocked(rect r)
{
	return false;	
}

void Map::ResizeChildren()
{

}

void Map::SetCameraPosition(point2d p, bool centered)
{
	mCameraPosition = p;
	if (centered) //Offset the camera around our center point
	{
		mCameraPosition.x -= (Width() / 2);
		mCameraPosition.y -= (Height() / 2);
	}
}

void Map::SetCameraPosition(rect r, bool centered) 
{
	point2d p(r.x, r.y);	
	if (centered) 
	{
		p.x += (r.w / 2);
		p.y += (r.h / 2);
	}
	
	SetCameraPosition(p, centered);
}

rect Map::GetCameraPosition()
{
	return rect(mCameraPosition.x, mCameraPosition.y, Width(), Height());
}

void Map::OffsetCamera(sShort offsetX, sShort offsetY)
{
	mCameraPosition.x += offsetX;
	mCameraPosition.y += offsetY;
}

void Map::OffsetCamera(point2d p)
{
	OffsetCamera(p.x, p.y);
}

void Map::UpdateCamera(uLong ms)
{
	//If we have any destinations, pan toward the oldest
	if (!mCameraDestinationStack.empty())
	{
	/*	sShort dx, dy;
		
		dx = mCameraDestinationStack.at(0).x - (mCameraPosition.x );//+ (Width() / 2));
		dy = mCameraDestinationStack.at(0).y - (mCameraPosition.y );//+ (Height() / 2));
	
		double theta = (dx == 0) ? 0 : tan(dy/dx);
		dx = static_cast<sShort>((double)mCameraSpeed * cos(theta));
		dy = static_cast<sShort>((double)mCameraSpeed * sin(theta));
	
		//auto correct once we get close
		if (mCameraPosition.x < mCameraDestinationStack.at(0).x 
			&& mCameraPosition.x + dx >= mCameraDestinationStack.at(0).x)
			mCameraPosition.x = mCameraDestinationStack.at(0).x;
		else
			mCameraPosition.x += dx;
			
		if (mCameraPosition.y < mCameraDestinationStack.at(0).y 
			&& mCameraPosition.y + dy >= mCameraDestinationStack.at(0).y)
			mCameraPosition.y = mCameraDestinationStack.at(0).y;
		else
			mCameraPosition.y += dy;
			
		//re-center it again
	//	mCameraPosition.x -= (Width() / 2);
	//	mCameraPosition.y -= (Height() / 2);

		if (mCameraPosition.x == mCameraDestinationStack.at(0).x 
			&& mCameraPosition.y == mCameraDestinationStack.at(0).y)
		{
			mCameraDestinationStack.erase(mCameraDestinationStack.begin());
			MessageData md("CAMERA_PANNED");
			messenger.Dispatch(md, this);
			
			if (mCameraDestinationStack.empty())
			{
				MessageData md("CAMERA_PANCLEAR");
				messenger.Dispatch(md, this);	
			}
		}*/
	}
	else
	{
		point2d p = GetCameraFollowOffset();
		Entity* e = GetCameraFollow();
		
		//If we're following, track their position.
		if (e)
		{
			p.x += e->mPosition.x;
			p.y += e->mPosition.y;
			SetCameraPosition( p, true);
		}
			
		if (mStopCameraAtMapEdge)
			_constrainCameraToMap();
	}
}

bool Map::IsRectInCamera(rect mapRect)
{
	return areRectsIntersecting(mapRect, GetCameraPosition());
}

rect Map::ToCameraPosition(rect screenRect)
{
	rect r = GetScreenPosition();
	return rect(screenRect.x + mCameraPosition.x - r.x, 
				screenRect.y + mCameraPosition.y - r.y,
				screenRect.w, screenRect.h);
}

rect Map::ToScreenPosition(rect mapRect)
{
	rect r = GetScreenPosition();
	return rect(mapRect.x - mCameraPosition.x + r.x, 
				mapRect.y - mCameraPosition.y + r.y, 
				mapRect.w, mapRect.h);
}

void Map::_constrainCameraToMap() 
{
	_constrainCameraX();
	_constrainCameraY();
}

void Map::_constrainCameraX()
{	
	//If the map width is smaller than our camera, center map on camera
	if (mWidth < Width() && mWidth != 0)
	{
		mCameraPosition.x = -(Width() - mWidth) / 2;
	}
	else //Constrain X to map edges
	{
		if (mCameraPosition.x + Width() > mWidth && mWidth != 0) 
			mCameraPosition.x = mWidth - Width();
			
		if (mCameraPosition.x < 0) 
			mCameraPosition.x = 0;
	}
}

void Map::_constrainCameraY()
{
	//If the map height is smaller than our camera, center map on camera
	if (mHeight < Height() &&  mHeight != 0)
	{
		mCameraPosition.y = -(Height() - mHeight) / 2;
	} 
	else //Constrain Y to map edges
	{ 
		if (mCameraPosition.y + Height() > mHeight && mHeight != 0) 
			mCameraPosition.y = mHeight - Height();
	
		if (mCameraPosition.y < 0) 
			mCameraPosition.y = 0;
	}
}

void Map::AddCameraDestination(point2d p)
{
	mCameraDestinationStack.push_back(p);
}

void Map::SetFlag(string flag, string value)
{
	flag = base64_encode(flag.c_str(), flag.length());
	value = base64_encode(value.c_str(), value.length());
	mFlags[flag] = value;
}

string Map::GetFlag(string flag)
{
	flag = base64_encode(flag.c_str(), flag.length());
	string value = mFlags[flag];
	
	if (value.empty())
		return value;
	
	return base64_decode(value.c_str(), value.length());
}

// Load our flags from PLAYERSAVE_FILENAME if we're in game mode
void Map::LoadFlags()
{
	if (!game)
		return;
		
	string flags;
	TiXmlElement* top = game->mPlayerData.mDoc.FirstChildElement();
	TiXmlElement* e;

	string elementId = "flags:" + mId;

	e = top->FirstChildElement(elementId.c_str());
	
	// no flags to load
	if (!e) 
		return; 
	
	flags = game->mPlayerData.GetText(e);
	
	vString v;
	explode(&v, &flags, ",");
	
	for (int i = 0; i < v.size(); i+=2)
	{
		DEBUGOUT(v.at(i));
		if (v.size() <= i+1)
			break;
		DEBUGOUT("[" + v.at(i) + "] => " + v.at(i+1));
		mFlags[v.at(i)] = v.at(i+1); //since it's encrypted, set directly. Not through SetFlag()
	}
}

// Save our map flags to PLAYERSAVE_FILENAME. If we don't have any, and an element exists, it'll be removed
void Map::SaveFlags()
{
	if (!game)
		return;
		
	TiXmlElement* top = game->mPlayerData.mDoc.FirstChildElement();
	TiXmlElement* e;

	string elementId = "flags:" + mId;

	e = top->FirstChildElement(elementId.c_str());
	
	if (mFlags.empty()) //trash the element, don't add
	{
		if (e)
			top->RemoveChild(e);
		return;	
	}
	
	string flags;
	
	//Convert our map to a string
	for (std::map<string, string>::iterator it = mFlags.begin(); it != mFlags.end(); ++it) 
	{
		flags += it->first + ',';
		flags += it->second + ',';
	}	

	if (!e) // Add one
	{
		e = new TiXmlElement(elementId.c_str());
		top->LinkEndChild(e);
	}
	
	game->mPlayerData.SetText(e, flags);
}

