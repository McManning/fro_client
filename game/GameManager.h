
#ifndef _GAMEMANAGER_H_
#define _GAMEMANAGER_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"
#include "WorldLoader.h"

const int JOIN_INTERVAL_MS = (10*1000);

const char* const DIR_DEV = "dev/";
const char* const DIR_AVA = "assets/ava/";
const char* const DIR_PROFILE = "profile/";

const char* const PLAYERDATA_ENCRYPTION_KEY = "JV4872JP64BJHD4X2JX";
const char* const PLAYERDATA_FILE = "profile.save";

class Map;
class LocalActor;
class IrcNet;
//class LunemParty;
class GameManager : public Frame
{
  public:
	GameManager();
	~GameManager();

	void ResizeChildren();
	void SetPosition(rect r);

	void UpdateAppTitle();

	//TODO: Move elsewhere?
	Console* GetPrivateChat(string nick);

	void LoadTestWorld(string luafile);
	void LoadOnlineWorld(string id, point2d target = point2d(), string targetObjectName = "");

	void Process(uLong ms);
	void Render();
	void Event(SDL_Event* event);

	void GenerateDefaultPlayerData();
	void LoadPlayerData();
	void SavePlayerData();

	void ToggleHud(bool visible);
	
//	void EndPlayersDuelTurn();

	bool IsMapLoading() const { return mLoader &&  mLoader->m_state != WorldLoader::WORLD_ACTIVE && mLoader->m_state != WorldLoader::IDLE; };
	
	void UnloadMap();
	
	//void DisplayAchievement(string title);
	
	/*	Returns the total number of points earned so far in the achievement, or 0 if passed in invalid parameters
		(max < 1 or title is empty)
	*/
	//int EarnAchievement(string title, string desc, int max);
	
	Map* mMap;
	LocalActor* mPlayer;
	Console* mChat;
	IrcNet* mNet;
	WorldLoader* mLoader;
	//LunemParty* mParty;
	
	Frame* mHud;

	bool mShowJoinParts;
	bool mShowAddresses;
	string mStartingWorldId;
	
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
		MODE_DUEL,	
	};
	
	gameMode mGameMode;
	
	void ToggleGameMode(gameMode mode);
	void ShowInfoBar(string id, string msg, int duration, string imageFile = "");

/*	void EnableDuelMode();
	void DisableDuelMode();
	bool IsInDuel();
*/

	void HideChat();
	void ShowChat();

  private:
	void _buildHud();
	void _buildChatbox();
	void _hookCommands();
	//void _addNewAchievement(string title, string desc, int max);
};

extern GameManager* game;

#endif //_GAMEMANAGER_H_
