
#include "LocalActor.h"
#include "../map/Map.h"
#include "../game/GameManager.h"
#include "../game/Achievements.h"
#include "../core/io/FileIO.h"
#include "../core/net/IrcNet2.h"
#include "../core/net/DataPacket.h"
#include "../core/io/Crypt.h"
#include "../game/IrcNetListeners.h"
#include "../entity/Avatar.h"

uShort timer_playerActionBufferSend(timer* t, uLong ms)
{
	LocalActor* p = (LocalActor*)t->userData;
	if (p && p == game->mPlayer && game->mMap)
	{
		p->mNeedsToSendBuffer = true;
		//p->NetSendActionBuffer();
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
	mIsLocked = false;
	
	mActionBufferTimer = timers->Add("actsend", ACTION_BUFFER_SEND_DELAY, false,
									timer_playerActionBufferSend,
									NULL,
									this);
	
	string url = game->mPlayerData.GetParamString("avatar", "url");

	LoadAvatar("assets/default.png", "", 32, 64, 1000, false, false);
	SwapAvatars();

	if (!url.empty())
		LoadAvatar( url, game->mPlayerData.GetParamString("avatar", "pass"),
						game->mPlayerData.GetParamInt("avatar", "w"),
						game->mPlayerData.GetParamInt("avatar", "h"),
						game->mPlayerData.GetParamInt("avatar", "delay"),
						game->mPlayerData.GetParamInt("avatar", "loopstand"),
						game->mPlayerData.GetParamInt("avatar", "loopsit") );

	LoadFlags();
}

LocalActor::~LocalActor()
{
	if (timers)
		timers->Remove(mActionBufferTimer);
		
	SaveFlags();
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
		//mLastSavedPosition = mPosition;
		//mLastSavedSpeed = mSpeed;

		NetSendActionBuffer();
	}

	if (mIsLocked || IsMoving() || !mMap || !mActionBuffer.empty()) 
		return;

	if (!game->mChat->HasKeyFocusInTree() && !mMap->HasKeyFocus())
		return;

	//Could use gui->IsKeyDown(key) however this method is probably faster
	Uint8 *keystate = SDL_GetKeyState(NULL); //get a snapshot of the keyboard

	direction newDir;

	bool moving = true;
	//Calculate change of direction based on keys held
	if ( keystate[SDLK_UP] )
	{
		if ( keystate[SDLK_LEFT] )
			newDir = NORTHWEST;
		else if ( keystate[SDLK_RIGHT] )
			newDir = NORTHEAST;
		else
			newDir = NORTH;
	}
	else if ( keystate[SDLK_DOWN] )
	{
		if ( keystate[SDLK_LEFT] )
			newDir = SOUTHWEST;
		else if ( keystate[SDLK_RIGHT] )
			newDir = SOUTHEAST;
		else
			newDir = SOUTH;
	}
	else if ( keystate[SDLK_LEFT] )
	{
		newDir = WEST;
	}
	else if ( keystate[SDLK_RIGHT] )
	{
		newDir = EAST;
	}
	else
	{
		newDir = mDirection; //no movement keys down, do w/e the player currently has
		moving = false;
	}

	//jump!
	if ( keystate[SDLK_TAB] )
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
			if ( /*GetSpeed() == SPEED_RUN*/ keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT] )
			{
				AddToActionBuffer("r");
			}
			else
			{
				AddToActionBuffer("w"); 
			}
		}
		else //standing jump
		{
			AddToActionBuffer("s"); 
		}
	}
	else if (moving) //regular movement
	{
		if (keystate[SDLK_RCTRL] || keystate[SDLK_LCTRL])
		{
			if (mDirection != newDir || mAction != SIT)
			{
				SetDirection(newDir);
				AddToActionBuffer(string("s") + directionToChar(newDir));
			}
		}
		else
		{
			
			//Detect change in speed
			if ( keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT] )
			{
				if (mSpeed != SPEED_RUN)
				{
					AddToActionBuffer("r");
				}
			}
			else //it was let up
			{
				if (mSpeed != SPEED_WALK)
				{
					AddToActionBuffer("w");
				}
			}
			
			/*
			//Somewhat emulate SDL_KEYDOWN, but as a one-time event until let go.
			if (!keystate[SDLK_RSHIFT] && !keystate[SDLK_LSHIFT]) //let go
			{
				mShiftDown = false;
			}
			else //one of them is down
			{
				if (!mShiftDown) //first press, 
				{
					mShiftDown = true;
					
					//Toggle walk/run
					if (GetSpeed() == SPEED_RUN)
						AddToActionBuffer("w");
					else
						AddToActionBuffer("r");
				}
			}
			*/
			
			SetDirection(newDir);
			StepForward();
		}
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

bool LocalActor::LoadAvatar(string file, string pass, uShort w, uShort h, uShort delay, 
							bool loopStand, bool loopSit)
{
	PRINT("LocalActor::LoadAvatar");

	bool result = Actor::LoadAvatar(file, pass, w, h, delay, loopStand, loopSit);
	
	//only save remote files
	if (result && (file.find("http://", 0) == 0 || file.find("avy://", 0) == 0) )
	{	
		game->mPlayerData.SetParamString("avatar", "url", file);
		game->mPlayerData.SetParamString("avatar", "pass", pass);
		game->mPlayerData.SetParamInt("avatar", "w", w);
		game->mPlayerData.SetParamInt("avatar", "h", h);
		game->mPlayerData.SetParamInt("avatar", "delay", delay);
		game->mPlayerData.SetParamInt("avatar", "loopstand", loopStand);
		game->mPlayerData.SetParamInt("avatar", "loopsit", loopSit);
		game->SavePlayerData();
		
		achievement_FashionAddict();

		netSendAvatar(mLoadingAvatar);
	}
	
	PRINT("LocalActor::LoadAvatar end");
	
	return result;
}

void LocalActor::Render(uLong ms)
{
	Actor::Render(ms);
}

void LocalActor::AddToActionBuffer(string data)
{
	mActionBuffer += data;
	mOutputActionBuffer += data;
}

const int NMREPLY_RESET_MS = 3*1000;

void LocalActor::NetSendState(string targetNick, string header) //header VERSION $id #x #y #dir #action Avatar Stuff
{
	if (!game->mNet || game->mNet->GetState() != ONCHANNEL)
		return;
			
	if (header == "nm")
	{
		if ( timers->Find("resetNm") )
			return;

		timers->Add("resetNm", NMREPLY_RESET_MS, false, NULL, NULL);
	}
	
	DataPacket data(header);
	data.SetKey( game->mNet->GetChannel()->mEncryptionKey );
	
	data.WriteString(APP_VERSION);
	data.WriteString(mName);
	data.WriteInt(mPosition.x);
	data.WriteInt(mPosition.y);
	data.WriteInt(mDirection);
	data.WriteInt(mAction);

	//if our avatar loaded but has yet to swap, swap it to send properly.
	_checkLoadingAvatar();

	//Add our avatar to the outbound message
	if (mAvatar)
		mAvatar->Serialize(data);
	else if (mLoadingAvatar)
		mLoadingAvatar->Serialize(data);
		
#ifdef DEBUG
	//Output our analyzed packet data
	string out = "[To: " + targetNick + "] {" + data.mId + "} ";
	for (int i = 0; i < data.Size(); i++)
		out += data.ReadString(i) + " & ";
	console->AddMessage(out);
#endif

	if (targetNick.empty())
		game->mNet->MessageToChannel( data.ToString() );
	else
		game->mNet->Privmsg( targetNick, data.ToString() );
}

void LocalActor::NetSendActionBuffer() //mov #x #y $buffer
{
	if (mOutputActionBuffer.empty()) return;

	DEBUGOUT("Sending: (" + its(mLastSavedPosition.x) + "," + its(mLastSavedPosition.y)
				+ ") " + mOutputActionBuffer);
						
	if (game->mNet && game->mNet->GetState() == ONCHANNEL)
	{
		/*TODO: I don't like this atm. It appends speed to the front of the buffer
			since I can't manage to keep it synced up. Can't watch when shift is released
			and pressed all the time since it'll constantly do rwrwrw when typing in input,
			also can't auto-run all the time w/ remote clients because it'll be jerky. So
			it'll be here in the start of mov to ensure it's always set. */
			
		DataPacket data("mov");
		data.SetKey( game->mNet->GetChannel()->mEncryptionKey );
	
		data.WriteInt(mLastSavedPosition.x);
		data.WriteInt(mLastSavedPosition.y);
		data.WriteString( compressActionBuffer( ((mSpeed == SPEED_WALK) ? "w" : "r") + mOutputActionBuffer ) );
	
		game->mNet->MessageToChannel( data.ToString() );
	}
	
	mLastSavedPosition.x = mDestination.x;
	mLastSavedPosition.y = mDestination.y;

	mOutputActionBuffer.clear();

	//So we don't prematurely send again after a forced sending
	if (mActionBufferTimer)
		mActionBufferTimer->lastMs = gui->GetTick();
}

void LocalActor::NetSendAvatarMod()
{
	if (!GetAvatar() || !game->mNet || game->mNet->GetState() != ONCHANNEL)
		return;
	
	DataPacket data("mod");
	data.SetKey( game->mNet->GetChannel()->mEncryptionKey );
	
	data.WriteInt(GetAvatar()->mModifier);
	
	game->mNet->MessageToChannel( data.ToString() );
}

// Load our entity flags from our main save file
void LocalActor::LoadFlags()
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
void LocalActor::SaveFlags()
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
	
	if (game->mMap->mOfflineMode) //if we're running from an offline test map, use editor dir
		game->mLoader.LoadOfflineWorld(id, position, targetObjectName, false);
	else //online mode, use cache and download sources
		game->mLoader.LoadOnlineWorld(id, position, targetObjectName);
}




