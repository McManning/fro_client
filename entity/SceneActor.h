
#ifndef _SCENEACTOR_H_
#define _SCENEACTOR_H_

#include "Actor.h" 

/*
	Scene actors are the NPCs of the game (Part of the scene, etc). They wear avatars, are capable
	of moving around the map and interacting with the player. Eventually, pathfinding, waypoints,
	and complex scripting techniques will be introduced for these actors.
*/	
class SceneActor : public Actor
{
  public:
	SceneActor();
	~SceneActor();
};

#endif //_SCENEACTOR_H_
