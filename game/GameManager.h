
#ifndef _GAMEMANAGER_H_
#define _GAMEMANAGER_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"
#include "WorldLoader.h"

const int JOIN_INTERVAL_MS = (60*1000);

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

	void UpdateAppTitle();

	//TODO: Move elsewhere?
	Console* GetPrivateChat(string nick);

	void LoadTestWorld(string luafile);
	void LoadOnlineWorld(string id, point2d target = point2d(), string targetObjectName = "");

	void Process(uLong ms);
	void Render(uLong ms);
	void Event(SDL_Event* event);

	void GenerateDefaultPlayerData();
	void LoadPlayerData();
	void SavePlayerData();

	bool IsMapLoading() const { return mLoader &&  mLoader->m_state != WorldLoader::WORLD_ACTIVE && mLoader->m_state != WorldLoader::IDLE; };
	
	void UnloadMap();
	
	void DisplayAchievement(string title);
	
	/*	Returns the total number of points earned so far in the achievement, or 0 if passed in invalid parameters
		(max < 1 or title is empty)
	*/
	int EarnAchievement(string title, string desc, int max, string file);
	
	Map* mMap;
	LocalActor* mPlayer;
	Console* mChat;
	IrcNet* mNet;
	WorldLoader* mLoader;

	Frame* mHud;

	bool mShowJoinParts;
	bool mShowAddresses;
	string mQueuedMapId;
	
	string mWarpDestinationObject;
	point2d mWarpDestinationPoint;

	XmlFile mConfig; //global config file for game-related material
	XmlFile mPlayerData; //our game-specific data. Including inventory, stats, etc. 

	//login credentials. mPassword is an md5 value.
	string mUsername;
	string mPassword;
	
	enum gameMode
	{
		MODE_CHAT = 0,
		MODE_ACTION,	
	};
	
	gameMode mGameMode;
	
	void ToggleGameMode(gameMode mode);

  private:
	void _hookCommands();
	void _renderMapLoader(uLong ms);
	void _addNewAchievement(string title, string desc, int max, string file);

	Image* mLoaderImage;
};

extern GameManager* game;

#endif //_GAMEMANAGER_H_
