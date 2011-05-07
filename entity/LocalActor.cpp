
#include <lua.hpp>
#include "LocalActor.h"
#include "../map/Map.h"
#include "../game/GameManager.h"
#include "../core/io/FileIO.h"
#include "../core/net/IrcNet2.h"
#include "../core/net/DataPacket.h"
#include "../core/io/Crypt.h"
#include "../net/IrcNetSenders.h"
#include "../avatar/Avatar.h"

uShort timer_playerActionBufferSend(timer* t, uLong ms)
{
	LocalActor* p = (LocalActor*)t->userData;
	if (p && p == game->mPlayer && game->mMap)
	{
		p->mNeedsToSendBuffer = true;
		t->interval = p->mActionBufferSendDelayMs; //readjust if necessary
	}
	return TIMER_CONTINUE;
}

LocalActor::LocalActor()
	: Actor()
{
	mType = ENTITY_LOCALACTOR;
	mShadow = true;
	mLimitedAvatarSize = true;
	mNeedsToSendBuffer = false;
	mShiftDown = false;
	mManagerCanDeleteMe = false; // GameManager will delete it
	mCanChangeAvatar = true;
	mIsLocked = false;
	mActionBufferSendDelayMs = DEFAULT_ACTION_BUFFER_SEND_DELAY;
	
	mActionBufferTimer = timers->Add("actsend", mActionBufferSendDelayMs, false,
									timer_playerActionBufferSend,
									NULL,
									this);

	string url = game->mPlayerData.GetParamString("avatar", "url");

	LoadAvatar("assets/default.png", "", 32, 64, 1000, 0);
	SwapAvatars();

	if (!url.empty())
		LoadAvatar( url, game->mPlayerData.GetParamString("avatar", "pass"),
						game->mPlayerData.GetParamInt("avatar", "w"),
						game->mPlayerData.GetParamInt("avatar", "h"),
						game->mPlayerData.GetParamInt("avatar", "delay"),
						game->mPlayerData.GetParamInt("avatar", "flags") );

	LoadFlagsFromXml();

}

LocalActor::~LocalActor()
{
	if (timers)
		timers->Remove(mActionBufferTimer);
		
	SaveFlagsToXml();
}

bool LocalActor::ProcessMovement()
{	
	_checkInput();

	bool result = Actor::ProcessMovement();

	return result;
}

void LocalActor::PostMovement()
{
	_checkInput(); //handle a second time to see if we want to keep moving or not

	Actor::PostMovement();
}

void LocalActor::_checkInput()
{
	if (game->IsMapLoading())
		return;

	if (!IsMoving() && mNeedsToSendBuffer)
	{
		mNeedsToSendBuffer = false;	

		if (!mOutputActionBuffer.empty())
		{
			netSendActionBuffer();
					
			mLastSavedPosition.x = mDestination.x;
			mLastSavedPosition.y = mDestination.y;
		
			mOutputActionBuffer.clear();
		
			//So we don't prematurely send again after a forced sending
			if (mActionBufferTimer)
				mActionBufferTimer->lastMs = gui->GetTick();
		}
	}
	
	if (game->mGameMode != GameManager::MODE_ACTION)
		return;

	if (gui->GetDemandsFocus() || mIsLocked || IsMoving() || !mMap || !mActionBuffer.empty()) 
		return;

	if ((game->GetChat() && !game->GetChat()->HasKeyFocusInTree()) && !mMap->HasKeyFocus())
		return;

	//Could use gui->IsKeyDown(key) however this method is probably faster
	Uint8 *keystate = SDL_GetKeyState(NULL); //get a snapshot of the keyboard

	direction newDir;

	bool moving = true;
	//Calculate change of direction based on keys held
	if ( keystate[SDLK_w] || keystate[SDLK_UP] )
	{
		if ( keystate[SDLK_a] || keystate[SDLK_LEFT] )
			newDir = NORTHWEST;
		else if ( keystate[SDLK_d] || keystate[SDLK_RIGHT] )
			newDir = NORTHEAST;
		else
			newDir = NORTH;
	}
	else if ( keystate[SDLK_s] || keystate[SDLK_DOWN] )
	{
		if ( keystate[SDLK_a] || keystate[SDLK_LEFT] )
			newDir = SOUTHWEST;
		else if ( keystate[SDLK_d] || keystate[SDLK_RIGHT] )
			newDir = SOUTHEAST;
		else
			newDir = SOUTH;
	}
	else if ( keystate[SDLK_a] || keystate[SDLK_LEFT] )
	{
		newDir = WEST;
	}
	else if ( keystate[SDLK_d] || keystate[SDLK_RIGHT] )
	{
		newDir = EAST;
	}
	else
	{
		newDir = mDirection; //no movement keys down, do w/e the player currently has
		moving = false;
	}

	//If they pressed the shift key, toggle speed
	if ( (keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT]))
	{
		if (!mShiftDown)
		{
			//toggle speed
			if (GetSpeed() == SPEED_RUN)
				AddToActionBuffer("w");
			else
				AddToActionBuffer("r");
			
			mShiftDown = true;
		}
	}
	else if (mShiftDown) //it was let up
	{
		mShiftDown = false;
	}
	
	//Check if they try to sit
	if (keystate[SDLK_RCTRL] || keystate[SDLK_LCTRL])
	{
		if (GetDirection() != newDir || GetAction() != SIT)
		{
			SetDirection(newDir);
			AddToActionBuffer(string("s") + directionToChar(newDir));
		}
		moving = false;
	}

	//jump!
	if ( keystate[SDLK_SPACE] )
	{
		if (newDir != GetDirection())
		{
			AddToActionBuffer(string("t") + directionToChar(newDir));
			SetDirection(newDir);
		}
		
		AddToActionBuffer("j");
		
		//determine jump type
		if (moving)
		{
			if (GetSpeed() == SPEED_RUN)
				AddToActionBuffer("r");
			else
				AddToActionBuffer("w"); 
		}
		else //standing jump
		{
			AddToActionBuffer("s"); 
		}
	}
	else if (moving) //regular movement
	{
		SetDirection(newDir);
		StepForward();
	}
}

bool LocalActor::StepForward()
{
	if (!CanMove(mDirection, 16)) return false;

	string s;
	s += directionToChar(mDirection);
	AddToActionBuffer(s);

	return true;
}

bool LocalActor::LoadAvatar(string file, string pass, uShort w, uShort h, uShort delay, uShort flags)
{
	PRINT("LocalActor::LoadAvatar");

	bool result = Actor::LoadAvatar(file, pass, w, h, delay, flags);
	
	//only save remote files
	if (result && (file.find("http://", 0) == 0 || file.find("avy://", 0) == 0) )
	{	
		game->mPlayerData.SetParamString("avatar", "url", file);
		game->mPlayerData.SetParamString("avatar", "pass", pass);
		game->mPlayerData.SetParamInt("avatar", "w", w);
		game->mPlayerData.SetParamInt("avatar", "h", h);
		game->mPlayerData.SetParamInt("avatar", "delay", delay);
		game->mPlayerData.SetParamInt("avatar", "flags", flags);
		game->SavePlayerData();

		netSendAvatar(mLoadingAvatar);
	}
	
	PRINT("LocalActor::LoadAvatar end");
	
	return result;
}

// Give a more verbose error message
void LocalActor::AvatarError(int err)
{
    string msg;
    string file;
    	
	if (game && game->GetChat())
	{
    	if (mLoadingAvatar)
    		file = GetFilenameFromUrl(mLoadingAvatar->mUrl);
    	
    	switch (err)
    	{
    		case Actor::AVYERR_LOADFAIL:
    			msg = "\\c900 [Avatar Error] Failed to download " + file;
    			break;
    		case Actor::AVYERR_BADIMAGE:
    			msg = "\\c900 [Avatar Error] Could not read format of " + file;
    			break;
    		case Actor::AVYERR_SIZE:
    			msg = "\\c900 [Avatar Error] Invalid frame sizes of " + file;
    			break;
    		case Actor::AVYERR_CONVERT:
    			msg = "\\c900 [Avatar Error] Could not convert " + file + ". " + mLoadingAvatar->mError;
    			break;
    		default: break;
    	}
    	
    	game->GetChat()->AddMessage(msg);
    }
	
	Actor::AvatarError(err);
}


void LocalActor::Render()
{
	Actor::Render();
}

void LocalActor::AddToActionBuffer(string data)
{
	mActionBuffer += data;
	mOutputActionBuffer += data;
}

// Load our entity flags from our main save file
void LocalActor::LoadFlagsFromXml()
{
	string flags;
	TiXmlElement* top = game->mPlayerData.mDoc.FirstChildElement();
	TiXmlElement* e;

	e = top->FirstChildElement("flags");
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

// Save our entity flags to our main save file
void LocalActor::SaveFlagsToXml()
{
	string flags;
	//Convert our map to a string
	for (std::map<string, string>::iterator it = mFlags.begin(); it != mFlags.end(); ++it) 
	{
		flags += it->first + ',';
		flags += it->second + ',';
	}	

	TiXmlElement* top = game->mPlayerData.mDoc.FirstChildElement();
	TiXmlElement* e;

	e = top->FirstChildElement("flags");
	game->mPlayerData.SetText(e, flags);

}

void LocalActor::PrintFlags()
{
	string flag, value;
	for (std::map<string, string>::iterator it = mFlags.begin(); it != mFlags.end(); ++it) 
	{
		flag = base64_decode( it->first.c_str(), it->first.length() );
		value = base64_decode( it->second.c_str(), it->second.length() );
		
		console->AddMessage("[" + flag + "] => " + value);
	}
}

void LocalActor::Warp(string id, point2d position, string targetObjectName)
{
	if (!game->mMap)
		return;

	game->LoadOnlineWorld(id, position, targetObjectName);
}

void LocalActor::Warp(point2d pos)
{
	AddToActionBuffer('c' + its(pos.x) + '.' + its(pos.y) + '.');
	mLastSavedPosition = pos;	
}

/*	index - Index of the stack where our new value for the property should be */
int LocalActor::LuaSetProp(lua_State* ls, string& prop, int index)
{
	if (prop == "locked") mIsLocked = lua_toboolean(ls, index);
	else if (prop == "avylocked") mCanChangeAvatar = lua_toboolean(ls, index);
	else if (prop == "bufferdelay") mActionBufferSendDelayMs = (int)lua_tonumber(ls, index);
	else if (prop == "mod" && GetAvatar()) // override the mod prop to send to the network when changed
	{
		SetAvatarModifier( (int)lua_tonumber(ls, index) );
		netSendAvatarMod();
	}
	else return Actor::LuaSetProp(ls, prop, index);
	
	return 1;
}

int LocalActor::LuaGetProp(lua_State* ls, string& prop)
{
	if (prop == "locked") lua_pushboolean( ls, mIsLocked );
	else if (prop == "avylocked") lua_pushboolean( ls, mCanChangeAvatar );
	else if (prop == "bufferdelay") lua_pushnumber( ls, mActionBufferSendDelayMs );
	else return Actor::LuaGetProp(ls, prop);
	
	return 1;
}

