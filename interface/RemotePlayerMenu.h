
#ifndef _REMOTEPLAYERMENU_H_
#define _REMOTEPLAYERMENU_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"

const int BEAT_INTERVAL_MS = 10000;

class RemoteActor;
class Map;
class RemotePlayerMenu : public Frame
{
  public:
	RemotePlayerMenu(Map* map, RemoteActor* linked);
	~RemotePlayerMenu() {};
	
	RemoteActor* mLinked;
	Map* mMap;
};

#endif //_REMOTEPLAYERMENU_H_




