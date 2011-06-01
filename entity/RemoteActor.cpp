
#include <lua.hpp>
#include "RemoteActor.h"
#include "../avatar/Avatar.h"
#include "../core/net/DataPacket.h"
#include "../map/Map.h"
#include "../game/GameManager.h"
#include "../net/IrcNetSenders.h"

RemoteActor::RemoteActor()
	: Actor()
{
	mType = ENTITY_REMOTEACTOR;
	mShadow = true;
	mLimitedAvatarSize = true;
	mBlocked = false;
	SetAfk(false);
	
	LoadAvatar("assets/default.png", "", 32, 64, 1000, 0);
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
		LoadAvatar("assets/blocked.png", "", 48, 48, 1000, 0);
	}
	else
	{
		netSendRequestAvatar(GetName());
	}
}

void RemoteActor::ReadAvatarFromPacket(DataPacket& data)
{
	int w, h, delay = 1000, flags = 0;
	string url, pass;
	int modifier;

	url = data.ReadString();
	w = data.ReadChar();
	h = data.ReadChar();
	delay = data.ReadInt();
	flags = data.ReadInt();
	modifier = data.ReadChar();
	pass = data.ReadString();
		
	if (LoadAvatar(url, pass, w, h, delay, flags))
		mLoadingAvatar->mModifier = modifier;
}

bool RemoteActor::LoadAvatar(string file, string pass, uShort w, uShort h, uShort delay, uShort flags)
{
	bool result = false;

	// only allow local files to be loaded if blocked
	if (IsBlocked() && file.find("assets", 0) != 0)
		return false;

	// only allow remote files, composite avatars, or local files, for remote actors
	if ( file.find("http://", 0) == 0 || file.find("avy://", 0) == 0 || file.find("assets", 0) == 0)
	{	
		result = Actor::LoadAvatar(file, pass, w, h, delay, flags);
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
	else if (prop == "hostmask") lua_pushstring( ls, mHostmask.c_str() );
	else return Actor::LuaGetProp(ls, prop);

	return 1;
}

