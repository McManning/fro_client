
#ifndef _WORLDLOADER_H_
#define _WORLDLOADER_H_

#include "../core/Core.h"

class WorldLoader
{
  public:
	WorldLoader() { mState = IDLE; };
	~WorldLoader() { };
	
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
	
	/*	Destroy our current map, leave it's channel, and download config for a new one */
	void LoadOnlineWorld(string id, point2d target, string targetObjectName = "");

	void LoadOfflineWorld(string id, point2d target, string targetObjectName = "", bool useCache = false);

	/*	Send request for world xml to master and await response */
	void _downloadConfig();

	void _reloadPreviousMap();

	/*	Load map xml, Create an appropriate Map type based on information in the XML.
		Then let Map::LoadProperties() and Map::QueueResources() have their way with our xml data.
		If this is an offline map, skip QueueResources and go straight to the world building phase.
	*/
	void _loadConfig();

	/*	Increment the amount of resources completed. If we finished them all, build the world */
	void _resourceDownloadCompleted(string url, string file);

	/*	Something went wrong. Cancel whole load */
	void _resourceDownloadFailed(string url, string file);

	/*	Called after we have all the necessary resources locally. This'll try to load them all into memory, configure, copy, move, etc. */
	void _buildWorld();

	/*	Add our character to the world, clean whatever up, and try to join the channel if online. 
		If offline, will set mState to WORLD_READY. Otherwise, WORLD_READY will be set after we successfully
		join the channel.
	*/
	void _joinWorld();

	/*	Bring birth to the world. Everything is okay, and the world is to be displayed. Called from GameManager when mState == WORLD_READY */
	void ActivateWorld();

	void _cancelLoad();

	/*	Add our player to the world, and set our position based on how we wanted to warp in */
	void _syncPlayerWithWorld();
	
	/*	Returns a value between 0.0 and 1.0 symbolizing our percent completed. */
	double Progress();
	
	void SetState(loaderState state);

	loaderState mState;
	
	string mId;
	point2d mWarpDestinationPoint;
	string mWarpDestinationObject;
	
	point2d mPreviousPosition;
	string mPreviousWorld;
	
	uShort mTotalResources;
	uShort mCompletedResources;
	
	bool mOnline; //steps will change based on this value.
	bool mUseCache; //true if we want to load our resources from cache, and get online resources
	bool mLoadingPrevious; //are we trying to load the previous map after a failure? (Prevents endless-looping if the previous also failed)
};

void callback_mapResourceDownloadSuccess(downloadData* data);
void callback_mapResourceDownloadFailure(downloadData* data);

#endif //_WORLDLOADER_H_



