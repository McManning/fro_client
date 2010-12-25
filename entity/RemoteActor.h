
#ifndef _REMOTEACTOR_H_
#define _REMOTEACTOR_H_

#include "Actor.h" 

class DataPacket;
class RemoteActor : public Actor
{
  public:
	RemoteActor();
	~RemoteActor();
	
	void Render();

	void ReadAvatarFromPacket(DataPacket& data);
	
	bool LoadAvatar(string file, string pass, uShort w, uShort h, uShort delay, 
							bool loopStand, bool loopSit);
	
	// Set to true if this player is considered "away from keyboard"
	void SetAfk(bool b) { mAfk = b; };
	
	void SetBlocked(bool b);
	bool IsBlocked() const { return mBlocked; };
	
	int LuaSetProp(lua_State* ls, string& prop, int index);
	int LuaGetProp(lua_State* ls, string& prop);
	
	bool mBlocked; //can we hear/see this player?
	bool mAfk;
};

#endif //_REMOTEACTOR_H_
