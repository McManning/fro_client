
#ifndef _REMOTEPLAYERMENU_H_
#define _REMOTEPLAYERMENU_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"

#define BEAT_INTERVAL_MS 10000

/*
	A right click menu for players. It's a row of buttons, displayed to the left of the linked player
	Each one will be an image button, vertical row. When the player hits a key or clicks outside this 
	panel, it will delete itself.
	
	ALSO NOTE: Should add a property to widgets, mTemporary, that auto deletes itself when
	an action occures outside of that widget. This way it'll be easier to make temp objects
	
	ALSO ANOTHER NOTE: There should be a clientside blocklist, that lists addresses of players
	who are auto-blocked.
	
	MENU:
		Block/Unblock Chat
		Block/Unblock Avatar
		Info (Whois)
		Beat with random item
		Open Private Message Window
		
	This'll be a global handler. On any mousebutton up, key down/up, it'll delete itself. 
	The focused button will get the event before the globals are called, so it'll get the event, do it's stuff, and wipe itself.
	We could also have multiple menus if we have too much data. One on the left of the player, one on the right. (Or hell, could do a circle around them..)
	Or.. one big invisble frame covering the player. Then left controls & right controls. (I like that idea)
*/
class Actor;
class Map;
class RemotePlayerMenu : public Frame 
{
  public:
	RemotePlayerMenu(Map* map, Actor* linked);
	~RemotePlayerMenu();
	
	void Render(uLong ms);
	void Event(SDL_Event* event);

	Actor* mLinked; //TODO: Change type later
	Map* mMap;
};

#endif //_REMOTEPLAYERMENU_H_
