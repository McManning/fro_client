
/*
 * Copyright (c) 2011 Chase McManning
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights 
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 */


#ifndef _GAMEMANAGER_H_
#define _GAMEMANAGER_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"
#include "../core/io/KeyValueMap.h"
#include "WorldLoader.h"

const int JOIN_INTERVAL_MS = (10*1000);

const char* const DIR_DEV = "dev/";
const char* const DIR_AVA = "assets/ava/";
const char* const DIR_PROFILE = "profile/";

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

	void LoadUserData();

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
	IrcNet* mNet;
	WorldLoader* mLoader;
	//LunemParty* mParty;

	bool mShowJoinParts;
	bool mShowAddresses;
	
	string mWarpDestinationObject;
	point2d mWarpDestinationPoint;

	KeyValueMap mUserData; // holds our map keys, game config, etc. 

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

    Console* GetChat();

  private:
	//void _addNewAchievement(string title, string desc, int max);
};

extern GameManager* game;

#endif //_GAMEMANAGER_H_
