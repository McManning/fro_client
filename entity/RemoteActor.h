
#ifndef _REMOTEACTOR_H_
#define _REMOTEACTOR_H_

#include "Actor.h" 

class DataPacket;
class RemoteActor : public Actor
{
  public:
	RemoteActor();
	~RemoteActor();
	
	void Render(uLong ms);

	void ReadAvatarFromPacket(DataPacket& data, uShort startOffset);
	
	bool LoadAvatar(string file, string pass, uShort w, uShort h, uShort delay, 
							bool loopStand, bool loopSit);
	
	bool mMuted; //can we hear this player?
};

#endif //_REMOTEACTOR_H_
