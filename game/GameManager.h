
#ifndef _GAMEMANAGER_H_
#define _GAMEMANAGER_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"
#include "WorldLoader.h"

const int JOIN_INTERVAL_MS = (60*1000);

const char* const GAME_CONFIG_FILENAME = "assets/game.cfg";

const char* const DIR_ENTITIES = "entities/";
const char* const DIR_SCRIPTS = "scripts/";
const char* const DIR_MAPS = "maps/";
const char* const DIR_EDITOR = "editor/";
const char* const DIR_AVA = "ava/";

const char* const PLAYERDATA_ENCRYPTION_KEY = "JV4872JP64BJHD4X2JX";
const char* const PLAYERDATA_FILE = "player.save";

class Map;
class LocalActor;
class IrcNet;
class GameManager : public Frame
{
  public:
	GameManager(bool forceLogin);
	~GameManager();

	void ResizeChildren();
	void SetPosition(rect r);

	void CreateHud();
	void ToggleHudSubMenu(string id);

	void UpdateAppTitle();

	//TODO: Move elsewhere?
	Console* GetPrivateChat(string nick);

	void Process(uLong ms);
	void Render(uLong ms);
	void Event(SDL_Event* event);

	void GenerateDefaultPlayerData();
	void LoadPlayerData();
	void SavePlayerData();

	bool IsMapLoading() { return mLoader.mState != WorldLoader::WORLD_ACTIVE && mLoader.mState != WorldLoader::IDLE; };
	
	void UnloadWorld();
	
	void DisplayAchievement(string title);
	
	/*	Returns the total number of points earned so far in the achievement, or 0 if passed in invalid parameters
		(max < 1 or title is empty)
	*/
	int EarnAchievement(string title, string desc, int max, string file);
	
	Map* mMap;
	LocalActor* mPlayer;
	Console* mChat;
	
	IrcNet* mNet;
	
	WorldLoader mLoader;
	
	string mMasterUrl;	
	bool mShowJoinParts;
	bool mShowAddresses;
	string mQueuedMapId;
	
	string mWarpDestinationObject;
	point2d mWarpDestinationPoint;

	XmlFile mConfig; //global config file for game-related material
	XmlFile mPlayerData; //our game-specific data. Including inventory, stats, etc. 

	Frame* mHudControls; 
	
	//Sub-Hud menus
	Frame* mFrameSystem;
	Frame* mFrameTools;
	Frame* mFrameUser;
	
	//login credentials. mPassword is an md5 value.
	string mUsername;
	string mPassword;

  private:
	void _renderMapLoader(uLong ms);
	void _addNewAchievement(string title, string desc, int max, string file);

	Image* mLoaderImage;
};

extern GameManager* game;

#endif //_GAMEMANAGER_H_
