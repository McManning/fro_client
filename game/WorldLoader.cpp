
#include "WorldLoader.h"
#include "GameManager.h"
#include "../entity/LocalActor.h"
#include "../map/CollectionMap.h"
#include "../core/net/IrcNet2.h"
#include "../core/widgets/MessagePopup.h"
#include "../core/io/FileIO.h"
#include "../interface/Userlist.h"
#include "../interface/LoginDialog.h"

void callback_mapResourceDownloadSuccess(downloadData* data)
{
	WorldLoader* loader = (WorldLoader*)data->userData;
	
	if (loader)
		loader->_resourceDownloadCompleted(data->url, data->filename);
}

void callback_mapResourceDownloadFailure(downloadData* data) 
{
	WorldLoader* loader = (WorldLoader*)data->userData;
	
	if (data->errorCode == DEC_BADHASH)
		console->AddMessage(data->url + " failed hash check.");
	else
		console->AddMessage(data->url + " failed with error code: " + its(data->errorCode));
	
	if (loader)
		loader->_resourceDownloadFailed(data->url, data->filename);
}

void WorldLoader::LoadOnlineWorld(string id, point2d target, string targetObjectName)
{
	_cancelLoad();
	SetState(IDLE);
	
	if (!game->mNet->IsConnected())
	{
		game->mChat->AddMessage("\\c900* Not connected!");
		return;
	}
	
	mId = id;
	mWarpDestinationPoint = target;
	mWarpDestinationObject = targetObjectName;
	mOnline = true;
	mUseCache = true;
	game->mQueuedMapId = id;
	mLoadingPrevious = false;

	game->mNet->PartChannel();
	
	_downloadConfig();
}

void WorldLoader::LoadOfflineWorld(string id, point2d target, string targetObjectName, bool useCache)
{		
	//if we're going from online->offline, get out of the online world
	game->mNet->PartChannel();
		
	_cancelLoad();
	SetState(IDLE);
	
	mId = id;
	mWarpDestinationPoint = target;
	mWarpDestinationObject = targetObjectName;
	mOnline = false;
	mLoadingPrevious = false;
	mUseCache = useCache;

	if (useCache)
		_downloadConfig();
	else
		_loadConfig();
}

void callback_mapXmlDownloadSuccess(downloadData* data)
{
	game->mLoader._loadConfig();
}

void callback_mapXmlDownloadFailure(downloadData* data)
{
	new MessagePopup("", "Map Load Error", "Failed to get config");
	game->mLoader._cancelLoad();
}

void WorldLoader::_downloadConfig()
{
	if (mId.empty())
	{
		new MessagePopup("", "Map Load Error", "Invalid World ID");
		_cancelLoad();
		return;
	}

	//generate the request url
	string url = game->mMasterUrl;
	url += "master.php?id=" + htmlSafe(mId);
	url += "&nick=" + htmlSafe(game->mPlayer->mName);
	
	if (!game->mUsername.empty())
		url += "&user=" + htmlSafe(game->mUsername) + "&pass=" + htmlSafe(game->mPassword);

	if (!mOnline)
		url += "&offline=1";
	
	if (!mPreviousWorld.empty())
	{
		url += "&old=" + htmlSafe(mPreviousWorld);
		url += "&x=" + its(mPreviousPosition.x);
		url += "&y=" + its(mPreviousPosition.y);
	}
	
	//target information
	url += "&tx=" + its(mWarpDestinationPoint.x);
	url += "&ty=" + its(mWarpDestinationPoint.y);
	url += "&to=" + htmlSafe(mWarpDestinationObject);
	
	game->mChat->AddMessage("\\c139* Sending request to master server for " + mId);
	DEBUGOUT(url);

	string file = DIR_CACHE + string("map.xml");
	
	//back up our map in case we need to reload it
	if (fileExists(file))
		copyFile(file, file + ".previous");
	
	if (!downloader->QueueDownload(url, file, 
									this, 
									callback_mapXmlDownloadSuccess, 
									callback_mapXmlDownloadFailure,
									true, true))
	{
		game->mChat->AddMessage("\\c900 ! Request failed");
		_cancelLoad();
		return;
	}
	
	SetState(GETTING_CONFIG);
}

/*
void WorldLoader::_reloadPreviousMap()
{
	mLoadingPrevious = true;
	
	string dst = DIR_CACHE + string("map.xml");
	string src = dst + ".previous";
	
	if (!fileExists(src))
		return;

	copyFile(src, dst);
	
	//in case we screwed up while already on the channel
	game->mNet->PartChannel();
	
	//skip _downloadConfig and go right to loading it back up. 
	_loadConfig();
}*/

void WorldLoader::_resourceDownloadCompleted(string url, string file)
{
	mCompletedResources++;
	
	//if we downloaded all our resources, proceed onto the next phase of load
	if (mCompletedResources == mTotalResources)
	{
		_buildWorld();
	}
}

void WorldLoader::_resourceDownloadFailed(string url, string file)
{
	new MessagePopup("", "Map Load Error", "Failed to get resource: " + url);
	_cancelLoad();
}

void WorldLoader::_buildWorld()
{
	SetState(BUILDING_WORLD);
	
	Map* map = game->mMap;
	
PRINT("_buildWorld 0");

	// Load in our persistant flags
	map->LoadFlags();

PRINT("_buildWorld 1");
	
	if ( !map->LoadScripts() )
	{
		new MessagePopup("", "Script Load Error", map->GetLoadError());
		_cancelLoad();
		return;
	}
	
PRINT("_buildWorld 2");
		
	if ( !map->LoadLayout() )
	{
		new MessagePopup("", "Layout Load Error", map->GetLoadError());
		_cancelLoad();
		return;
	}

PRINT("_buildWorld 3");

	_joinWorld();

PRINT("_buildWorld 4");
}

void WorldLoader::_joinWorld()
{
	SetState(JOINING_WORLD);

	_syncPlayerWithWorld();
	
	if (mOnline)
	{
		if (game->mNet->GetState() == DISCONNECTED)
		{
			new MessagePopup("", "World Load Error", "Disconnected while waiting for join");
			_cancelLoad();
			return;
		}

		if (!game->mMap->mChannelId.empty())
			game->mNet->JoinChannel(game->mMap->mChannelId); //will set state to WORLD_READY after completion
	}
	else
	{
		SetState(WORLD_READY);
	}
}

void WorldLoader::ActivateWorld()
{
	game->Add(game->mMap);

	SetState(WORLD_ACTIVE);
}

void WorldLoader::_cancelLoad()
{
	//we were cancelled while everything was just fine, and we weren't loading, just kill games map and return
	if (mState == WORLD_ACTIVE)
	{
		if (game->mMap)
		{
			game->mMap->Die();
			
			// do a little post-deletion cleanup (kinda hacky.. but eh)
			for (int i = 0; i < game->mMap->mScripts.size(); i++)
			{
				SAFEDELETE(game->mMap->mScripts.at(i));	
			}
			game->mMap->mScripts.clear();
		}
		game->mMap = NULL;
		return;
	}

	//Else, we gotta deal with cleaning up the loaders mess.

	SAFEDELETE(game->mMap);
	game->mPlayer->mMap = NULL;

	SetState(FAILED);
	
	//if we have a previous map (and this _cancelLoad didn't happen while trying to load a previous map), load it.
	/*if (!mLoadingPrevious)
	{
		_reloadPreviousMap();
	}*/
	
	//instead, let them /join somewhere else instantly (remove timer)
	//If we re-add _reloadPrevious, this cannot be used.
	timers->Remove("joinwait");
	
	//if they're not on a server, and there's no map, bring login back up
/*	if (!game->mMap && (!game->mNet || !game->mNet->IsConnected()))
	{
		if (!loginDialog)
			new LoginDialog();
	}*/
	
	TODO: Downloader is still active!
}

void WorldLoader::_syncPlayerWithWorld()
{
	LocalActor* p = game->mPlayer;
	p->mMap = game->mMap;
	p->ClearActionBuffer();
	
	p->mMap->AddEntity(p, ENTITYLEVEL_USER);
	p->mMap->SetCameraFollow(p);
	
	//spawn our player in a different position, based on the destination type
	if (!mWarpDestinationObject.empty()) //warp us to the center of this object
	{
		Entity* e = p->mMap->FindEntityByName(mWarpDestinationObject, ENTITY_STATICOBJECT);
		
		if (!e) //object not found, warp to default spawn
			p->SetPosition( p->mMap->mSpawnPoint );
		else
			p->SetPosition( e->GetPosition() );
	}
	else if ( !isDefaultPoint2d(mWarpDestinationPoint) ) //warp us to a specified coordinate
	{
		p->SetPosition( mWarpDestinationPoint );
	}
	else //warp to default spawn
	{
		p->SetPosition( p->mMap->mSpawnPoint );
	}

	if (userlist)
		userlist->AddNick(p->mName);
}

double WorldLoader::Progress()
{
	if (mState <= FAILED)
		return 0.0;
	
	if (mState >= WORLD_READY)
		return 1.0;

	if (mState == GETTING_CONFIG)
		return 0.1;
		
	if (mState >= GETTING_RESOURCES && mState < JOINING_WORLD)
		return 0.1 + (0.8 / mTotalResources) * mCompletedResources;
	
	if (mState == JOINING_WORLD)
		return 0.9;

	return 3.14159;
}

void WorldLoader::SetState(loaderState state)
{
	mState = state;
	
	//Disable the chatbox while loading
	if (mState <= FAILED || mState == WORLD_ACTIVE)
		game->mChat->SetVisible(true);
	else
		game->mChat->SetVisible(false);
}



