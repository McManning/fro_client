
#include "RemoteActor.h"
#include "Avatar.h"
#include "../core/net/DataPacket.h"
#include "../map/Map.h"
#include "../game/GameManager.h"

RemoteActor::RemoteActor()
	: Actor()
{
	mType = ENTITY_REMOTEACTOR;
	mShadow = true;
	mLimitedAvatarSize = true;
	mMuted = false;
	
	LoadAvatar("assets/default.png", "", 32, 64, 1000, false, false);
	SwapAvatars();
}

RemoteActor::~RemoteActor()
{

}

void RemoteActor::Render(uLong ms)
{
	Actor::Render(ms);
}

void RemoteActor::ReadAvatarFromPacket(DataPacket& data, uShort startOffset)
{
	uShort w, h, delay = 1000;
	bool loopStand = true, loopSit = false;
	string url, pass;
	int modifier;
	
	if (data.Size() > startOffset + 5) //if we have avatar info, read it
	{
		url = data.ReadString(startOffset);
		w = data.ReadInt(startOffset+1);
		h = data.ReadInt(startOffset+2);

		if (data.Size() > startOffset+3)
			delay = data.ReadInt(startOffset+3);

		if (data.Size() > startOffset+4)
			loopStand = data.ReadInt(startOffset+4);

		if (data.Size() > startOffset+5)
			loopSit = data.ReadInt(startOffset+5);
			
		if (data.Size() > startOffset+6)
			modifier = data.ReadInt(startOffset+6);
			
		if (data.Size() > startOffset+7)
			pass = data.ReadString(startOffset+7);
			
		if (LoadAvatar(url, pass, w, h, delay, loopStand, loopSit))
			mLoadingAvatar->mModifier = modifier;
	}
}

bool RemoteActor::LoadAvatar(string file, string pass, uShort w, uShort h, uShort delay, 
							bool loopStand, bool loopSit)
{
	PRINT("RemoteActor::LoadAvatar");

	bool result = false;

	//only allow remote files, composite avatars, or local files, for remote actors
	if ( file.find("http://", 0) == 0 || file.find("avy://", 0) == 0 || file.find("assets", 0) == 0)
	{	
		result = Actor::LoadAvatar(file, pass, w, h, delay, loopStand, loopSit);
	}
	
	return result;
}

