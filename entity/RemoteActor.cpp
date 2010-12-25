
#include <lua.hpp>
#include "RemoteActor.h"
#include "Avatar.h"
#include "../core/net/DataPacket.h"
#include "../map/Map.h"
#include "../game/GameManager.h"
#include "../game/IrcNetListeners.h"

RemoteActor::RemoteActor()
	: Actor()
{
	mType = ENTITY_REMOTEACTOR;
	mShadow = true;
	mLimitedAvatarSize = true;
	mBlocked = false;
	SetAfk(false);
	
	LoadAvatar("assets/default.png", "", 32, 64, 1000, false, false);
	SwapAvatars();
}

RemoteActor::~RemoteActor()
{

}

void RemoteActor::Render()
{
	Actor::Render();
}

void RemoteActor::SetBlocked(bool b)
{
	mBlocked = b;
	//TODO: Store blocked address in a file, yadda yadda
	
	if (mBlocked)
	{
		//Change their avatar to the blocked form
		LoadAvatar("assets/blocked.png", "", 48, 48, 1000, false, false);
	}
	else
	{
		netSendRequestAvatar(mName);
	}
}

void RemoteActor::ReadAvatarFromPacket(DataPacket& data)
{
	int w, h, delay = 1000;
	bool loopStand = true, loopSit = false;
	string url, pass;
	int modifier;

	url = data.ReadString();
	w = data.ReadChar();
	h = data.ReadChar();
	delay = data.ReadInt();
	loopStand = data.ReadChar();
	loopSit = data.ReadChar();
	modifier = data.ReadChar();
	pass = data.ReadString();
		
	if (LoadAvatar(url, pass, w, h, delay, loopStand, loopSit))
		mLoadingAvatar->mModifier = modifier;
}

bool RemoteActor::LoadAvatar(string file, string pass, uShort w, uShort h, uShort delay, 
							bool loopStand, bool loopSit)
{
	PRINT("RemoteActor::LoadAvatar");

	bool result = false;

	// only allow local files to be loaded if blocked
	if (IsBlocked() && file.find("assets", 0) != 0)
		return false;

	// only allow remote files, composite avatars, or local files, for remote actors
	if ( file.find("http://", 0) == 0 || file.find("avy://", 0) == 0 || file.find("assets", 0) == 0)
	{	
		result = Actor::LoadAvatar(file, pass, w, h, delay, loopStand, loopSit);
	}
	
	return result;
}

/*	index - Index of the stack where our new value for the property should be */
int RemoteActor::LuaSetProp(lua_State* ls, string& prop, int index)
{
	if (prop == "blocked") SetBlocked( lua_toboolean(ls, index) );

	else return Actor::LuaSetProp(ls, prop, index);

	return 1;
}

int RemoteActor::LuaGetProp(lua_State* ls, string& prop)
{
	if (prop == "blocked") lua_pushboolean( ls, IsBlocked() );

	else return Actor::LuaGetProp(ls, prop);

	return 1;
}

