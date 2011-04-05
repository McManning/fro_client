
#ifndef _WORLDLOADER_H_
#define _WORLDLOADER_H_

#include "../core/widgets/Frame.h"

const char* const COMMON_LUA_INCLUSION = "scripts/main.lua";

class Label;
class WorldLoader : public Frame
{
  public:
	WorldLoader();
	~WorldLoader();
	
	typedef enum
	{
		IDLE = 0,
		FAILED, //load failure
		GETTING_CONFIG, //waiting for the worlds main xml to finish downloading
		GETTING_RESOURCES, //waiting for all resources to download and return success
		BUILDING_WORLD, //loading resources from disk and loading layout
		JOINING_WORLD, //waiting for irc channel to accept join 
		WORLD_READY, //waiting for GameManager to call ActivateWorld()
		WORLD_ACTIVE //the world is visible and the player can move around on it
	} loaderState;
	
	void LoadOnlineWorld(string id, point2d target = point2d(), string targetObjectName = "");
	
	/*	Load a world, using /dev/ as our cache. It assumes all resources exist locally and will
		not attempt to download any. 
	*/
	void LoadTestWorld(string id);
	
	/*
		Load the primary lua file of this map and tries to run the Build(); function 
		to actually construct the map. 
	*/
	void _buildWorld();
	
	void _finalize();
	
	/*	Called by GameManager once we have WORLD_READY state, 
		to actually display it and let the player interact with the world */
	void DisplayWorld();
	
	void _syncPlayerWithWorld();
	void _error(string msg);
	double Progress();
	void SetState(loaderState state);
	
	void Render();
	void UpdateStatusText();
	
	/*	Routines for loading online maps only */
	void _sendRequestForConfig();
	void _parseConfig();
	int _queueResources(string items);
	void _resourceDownloadSuccess(string url, string file);
	void _resourceDownloadFailure(string url, string file);

	loaderState m_state;
	
	point2d m_warpDestinationPoint;
	string m_sWarpDestinationEntityName;

	bool m_bTestMode; //is this map being loaded in a testing mode, or online

	int m_iCompletedResources;
	
	std::vector<string> m_vsResources;
	
	string m_sWorldName; // Set in PHASE A
	string m_sChannelName; // Set in PHASE A, accessed in PHASE D
	string m_sResourcesList; // Set in PHASE A, accessed in PHASE B

	point2d m_previousPosition;
	string m_sPreviousMap;

	Image* m_BackgroundImage;
	Label* m_StatusLabel;
};

#endif //_WORLDLOADER_H_
