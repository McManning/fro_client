
#include <lua.hpp>

#include "WorldLoader.h"
#include "GameManager.h"
#include "../entity/LocalActor.h"
#include "../map/BasicMap.h"
#include "../core/net/IrcNet2.h"
#include "../core/widgets/MessagePopup.h"
#include "../core/io/FileIO.h"
#include "../interface/Userlist.h"
#include "../interface/LoginDialog.h"

#include "../lua/MapLib.h"

#define ONLINE_ENABLED

WorldLoader::WorldLoader()
{
	PRINT("Creating WorldLoader");
	SetState(IDLE);
	PRINT("WorldLoader Done");
}

WorldLoader::~WorldLoader()
{
	// TODO: Some sort of cleanup if we're deleted mid-download!
}

void WorldLoader::LoadOnlineWorld(string id, point2d target, string targetObjectName)
{
#ifdef ONLINE_ENABLED
	SetState(IDLE);
	
	if (!game->mNet->IsConnected())
	{
		game->mChat->AddMessage("\\c900* Not connected!");
		return;
	}
	
	if (game->mMap) //save previous info if we got it
	{
		m_previousPosition = game->mPlayer->GetPosition();
		m_sPreviousMap = game->mMap->mId;
	}
	
	m_sWorldName = id;
	m_warpDestinationPoint = target;
	m_sWarpDestinationEntityName = targetObjectName;
	m_bTestMode = false;

	game->mNet->PartChannel();
	game->UnloadMap();
	
	_sendRequestForConfig();
#endif
}

/*	Load a world directly from the lua file. It assumes all resources exist locally */
void WorldLoader::LoadTestWorld(string luafile)
{		
	SetState(IDLE);

	m_sWorldName = luafile;
	m_sPrimaryLuaFile = luafile;
	m_bTestMode = true;
	
	//if we're going from online->offline, get out of the online world
	if (game->mNet)
		game->mNet->PartChannel();
		
	game->UnloadMap();
	
	_buildWorld();
}

/*	************************************
	PHASE A (online): Get config
		Send primary request to master, 
		parse return xml and configure
		load vars. Jump to PHASE B 
		if successful
	************************************ */
#ifdef ONLINE_ENABLED
void callback_masterResponse_Success(downloadData* data)
{
	WorldLoader* loader = (WorldLoader*)data->userData;
	if (loader)
		loader->_parseConfig();
}

void callback_masterResponse_Failure(downloadData* data)
{
	WorldLoader* loader = (WorldLoader*)data->userData;
	if (loader)
		loader->_error("Failed to download config");
}
	
void WorldLoader::_sendRequestForConfig()
{
	string url;
	
	if (m_sWorldName.empty())
	{
		_error("Empty m_sWorldName");
		return;
	}

	XmlFile xf;
	if (!xf.LoadFromFile("assets/connections.cfg"))
	{
		FATAL(xf.GetError());	
	}
	
	TiXmlElement* e = xf.mDoc.FirstChildElement();
	if (e)
		e = e->FirstChildElement("master");
	
	if (e)
		url = xf.GetText(e);
	
	if (url.empty() || url.find("http://", 0) != 0)
	{
		FATAL("Invalid master address");
	}

	//generate the request url
	url += "?id=" + htmlSafe(m_sWorldName);
	url += "&nick=" + htmlSafe(game->mPlayer->mName);
	
	if (!game->mUsername.empty())
		url += "&user=" + htmlSafe(game->mUsername) + "&pass=" + htmlSafe(game->mPassword);
/*
	if (!m_bOnline)
		url += "&offline=1";
	
	if (!mPreviousWorld.empty())
	{
		url += "&old=" + htmlSafe(mPreviousWorld);
		url += "&x=" + its(mPreviousPosition.x);
		url += "&y=" + its(mPreviousPosition.y);
	}
	
	//target information
	url += "&tx=" + its(m_warpDestinationPoint.x);
	url += "&ty=" + its(m_warpDestinationPoint.y);
	url += "&to=" + htmlSafe(m_sWarpDestinationEntityName);
*/	
	game->mChat->AddMessage("\\c139* Sending request to master server for " + m_sWorldName);
	DEBUGOUT(url);

	string file = DIR_CACHE + string("map.res");

	if (!downloader->QueueDownload(url, file, 
									this, 
									callback_masterResponse_Success, 
									callback_masterResponse_Failure,
									true, true))
	{
		_error("Config request fail");
		return;
	}
	
	SetState(GETTING_CONFIG);
}

/*	Parse the config, make sure it's good and all that. 
	Upon success, will jump to Phase B

	Response from master.php will take the following form (Completely constructed serverside)
	Requirements for construction:
		A mapid.res file that contains all the necessary resources.

	master:http://localhost/fro/worlds/
	channel:#fro.library
	entities/library/fire.png:bc7c421648faf4bcf4a25c22cd81962d
	entities/library/floor.jpg:6dd0b696f4a66a5087b1fe0cdb6bb951
	entities/library/overlay.png:b7bda3c471a8e87a0a393926e4f87899
	
	OR
	
	error:Some error message
*/
void WorldLoader::_parseConfig()
{
	string text;
	if (!fileToString(text, DIR_CACHE + string("map.res")))
	{
		_error("Failed to load map config");
		return;	
	}

	m_iTotalResources = 0;
	m_iCompletedResources = 0;
	SetState(GETTING_RESOURCES);
	
	//check for error:Some message
	if (text.find("error:", 0) == 0)
	{
		_error(text.substr(6));
		return;
	}
	
	if ( !_queueResources(text) )
	{
		_error("Malformed map config");
		return;
	}
}

/*	************************************
	PHASE B (online): Get resources
		Iterate through resources list, 
		download all. Once all are stored
		and verified locally, jump to PHASE C
	************************************ */

void callback_ResourceDownload_Success(downloadData* data)
{
	WorldLoader* loader = (WorldLoader*)data->userData;
	
	if (loader)
		loader->_resourceDownloadSuccess(data->url, data->filename);
}

void callback_ResourceDownload_Failure(downloadData* data) 
{
	WorldLoader* loader = (WorldLoader*)data->userData;
	
	if (data->errorCode == DEC_BADHASH)
		console->AddMessage(data->url + " failed hash check.");
	else
		console->AddMessage(data->url + " failed with error code: " + its(data->errorCode));
	
	if (loader)
		loader->_resourceDownloadFailure(data->url, data->filename);
}

/*
	Will queue up downloads for all the files mentioned in the list. In the following format:
	
	master:http://sybolt.com/drm/worlds/	Must be the first line. Will be the prefix when constructing the url of each download
											If this is not the first line, the file will be considered invalid and function returns 0
	channel:#fro.something					Sets m_sChannelName. Required, otherwise it won't connect to a channel once downloaded.
	/path/to/file.ext:MD5hash				Will compare /cache/path/to/file.ext's md5 with this value. 
											If different, delete & add to download as {master}/path/to/file.ext
	/path/to/file2.ext						No :hash will skip download if /cache/path/to/file2.ext exists
	[more files follow, each on a new line]
	
	If all files are successfully added, function will return 1
*/

int WorldLoader::_queueResources(string items)
{
	string master;
	string path;
	string hash;
	bool overwrite;
	vString v;
	size_t pos;
	
	if (items.empty())
		return 1;

	replace(&items, "\r", "");
	explode(&v, &items, "\n");

	for (int i = 1; i < v.size(); ++i)
	{
		//cleanup all whitespace
		for (pos = 0; pos < v.at(i).length(); ++pos)
		{
			if (isWhitespace(v.at(i).at(pos)))
			{
				v.at(i).erase(v.at(i).begin() + pos);
				--pos;
			}
		}

		if (v.at(i).find("channel:", 0) == 0)
		{
			m_sChannelName = v.at(i).substr(8);
		}
		else if (v.at(i).find("master:", 0) == 0)
		{
			master = v.at(i).substr(7);	
		}
		else if (!v.at(i).empty())
		{
			pos = v.at(i).find(":",0);
			if (pos == string::npos) //case of /path/to/file.ext, no hash. If it exists, it's good.
			{
				path = v.at(i);
				hash.clear();
				overwrite = false; //keep the old one, if it's there.
			}
			else
			{
				path = v.at(i).substr(0, pos);
				hash = v.at(i).substr(pos+1, v.at(i).length() - pos);
				overwrite = true; //will only download & overwrite if the old hash doesn't match the new one
			}
			
			//See if this is the first lua file we've encountered (if it has .lua extension). If so, consider it the primary
			if (m_sPrimaryLuaFile.empty() && path.find(".lua", 0) != string::npos)
			{
				console->AddMessage(" * Set Primary Script: " + m_sPrimaryLuaFile);
				m_sPrimaryLuaFile = path;
			}
			
			console->AddMessage(" * Queuing master:" + master + " path:" + path + " hash:" + hash);
			
			downloader->QueueDownload(master + path, DIR_CACHE + path, this,
										callback_ResourceDownload_Success,
										callback_ResourceDownload_Failure,
										overwrite, false, hash);
			++m_iTotalResources;
		}
	}
	
	return 1;
}

void WorldLoader::_resourceDownloadSuccess(string url, string file)
{
	++m_iCompletedResources;
	
	console->AddMessage("\\c990Completed: " + its(m_iCompletedResources) + " Total: " + its(m_iTotalResources));
	
	//if we downloaded all our resources, proceed onto the next phase of load
	if (m_iCompletedResources == m_iTotalResources)
	{
		_buildWorld();
	}
}

void WorldLoader::_resourceDownloadFailure(string url, string file)
{
	_error("Failed to get resource: " + url);
}
#endif //ONLINE_ENABLED

/*	************************************
	PHASE C: Construction
		Load primary lua file, run Build(), 
		jump to PHASE D if successful
	************************************ */
	
/*
	Load the primary lua file of this map and tries to run the Build() function 
	to actually construct the map. 
*/
void WorldLoader::_buildWorld()
{
	lua_State* ls = mapLib_OpenLuaState();

	string file;
	
	if (m_sPrimaryLuaFile.empty())
	{
		_error( "Primary lua file missing" );
		return;
	}
	
	if (!m_bTestMode)
		file = DIR_CACHE + m_sPrimaryLuaFile;
	else
		file = m_sPrimaryLuaFile; 

	// Load the file itself
	if ( luaL_dofile( ls, file.c_str() ) != 0 )
	{
		string err = lua_tostring(ls, -1);
		lua_close(ls);

		_error( "Lua parse error: " + err );
		return;
	}
	
	// Parsed right, now let's run our Build()!
	if ( !mapLib_luaCallBuild(ls) )
	{
		mapLib_CloseLuaState(ls);
		_error("Lua call to Build() failed");
		return;
	}
	
	if (!game->mMap)
	{
		mapLib_CloseLuaState(ls);
		_error("Lua Build() never created the map");
		return;
	}
	
	game->mMap->mLuaState = ls;
	
	//All parsing went well we assume, finalize it!
	_finalize();
}

/*	************************************
	PHASE D: Finalization
		Add player to the map, adjust camera, 
		display map, join channel (if online)
	************************************ */

void WorldLoader::_finalize()
{
	SetState(JOINING_WORLD);

	_syncPlayerWithWorld();
	
	if (m_bTestMode) //no need to wait for channel connection
	{
		SetState(WORLD_READY);
		return;
	}
	
	if (game->mNet->GetState() == DISCONNECTED)
	{
		_error("Disconnected while waiting for map");
		return;
	}
	
	//if we were actually provided a channel name to join, do so before displaying
	if (!m_sChannelName.empty()) 
		game->mNet->JoinChannel(m_sChannelName); //will set state to WORLD_READY after completion
	else
		SetState(WORLD_READY);
}

/*	Called by GameManager once we have WORLD_READY state, 
	to actually display it and let the player interact with the world */
void WorldLoader::DisplayWorld()
{
	game->Add(game->mMap);
	SetState(WORLD_ACTIVE);
	
	mapLib_CallDisplay(game->mMap->mLuaState); //let lua do whatever it wants
}

void WorldLoader::_syncPlayerWithWorld()
{
	LocalActor* p = game->mPlayer;
	p->mMap = game->mMap;
	p->SetLayer(EntityManager::LAYER_USER);
	p->ClearActionBuffer();
	
	p->mMap->AddEntity(p);
	p->mMap->SetCameraFollow(p);
	
	//spawn our player in a different position, based on the destination type
	if (!m_sWarpDestinationEntityName.empty()) //warp us to the center of this object
	{
		Entity* e = p->mMap->FindEntityByName(m_sWarpDestinationEntityName, ENTITY_STATICOBJECT);
		
		if (!e) //object not found, warp to default spawn
			p->SetPosition( p->mMap->mSpawnPoint );
		else
			p->SetPosition( e->GetPosition() );
	}
	else if ( !isDefaultPoint2d(m_warpDestinationPoint) ) //warp us to a specified coordinate
	{
		p->SetPosition( m_warpDestinationPoint );
	}
	else //warp to default spawn
	{
		p->SetPosition( p->mMap->mSpawnPoint );
	}

	if (userlist)
		userlist->AddNick(p->mName);
}

	
/*	************************************
	ERROR PHASE: Jumped to if error in any phase
		Cleanup whatever may be loaded.
	************************************ */

void WorldLoader::_error(string msg)
{
	//if we got to a point where the loader set game->mMap, need to nuke it
	if (game->mMap)
	{
		mapLib_CloseLuaState(game->mMap->mLuaState);
		SAFEDELETE(game->mMap); 
		game->mPlayer->mMap = NULL;
	}

	SetState(FAILED);

	//instead, let them /join somewhere else instantly (remove timer)
	timers->Remove("joinwait");
	
	//if they're not on a server, and there's no map, bring login back up
/*	if (!game->mMap && (!game->mNet || !game->mNet->IsConnected()))
	{
		if (!loginDialog)
			new LoginDialog();
	}*/
	
	//TODO: Downloader might still be active! Kill all linked downloads!
	
	new MessagePopup("", "Map Load Error", msg);
}
	
/*	************************************
	OTHER STUFF
	************************************ */
	
double WorldLoader::Progress()
{
	if (m_state <= FAILED || m_state > JOINING_WORLD)
		return 0.0;
	
	if (m_state >= WORLD_READY)
		return 1.0;

	if (m_state == GETTING_CONFIG)
		return 0.1;
		
	if (m_state >= GETTING_RESOURCES && m_state < JOINING_WORLD)
		return 0.1 + (0.8 / m_iTotalResources) * m_iCompletedResources;

	return 0.9; //m_state == JOINING_WORLD
}

void WorldLoader::SetState(loaderState state)
{
	m_state = state;
	
	if (game && game->mChat)
	{
		//Disable the chatbox while loading
		if (m_state <= FAILED || m_state == WORLD_ACTIVE)
			game->mChat->SetVisible(true);
		else
			game->mChat->SetVisible(false);
	}
}
