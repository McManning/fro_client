
#include <lua.hpp>

#include "MapLib.h"
#include "LuaCommon.h"

/* All sub libraries will basically be initialized from this library */
#include "EventsLib.h"
#include "TimersLib.h"
#include "SystemLib.h"
#include "EntityLib.h"
#include "PlayerLib.h"
#include "CameraLib.h"
#include "ActorLib.h"
#include "ConvoLib.h"
#include "GameLib.h"

#include "../game/GameManager.h"
#include "../map/Map.h"

/*	Calls __BUILD() in the lua script. Return 0 on error */
int mapLib_luaCallBuildWorld(lua_State* ls, bool testMode, string& worldName, std::vector<string>& reslist)
{
	lua_getglobal(ls, "__MAIN_BUILD");

	//if there isn't a function at the top of the stack, we failed to find it
	if (!lua_isfunction(ls, -1))
	{
		console->AddMessage("\\c900 * LUA __MAIN_BUILD Not Found");
		return 0;
	}

	lua_pushboolean(ls, testMode); // first parameter: test mode boolean
	lua_pushstring(ls, worldName.c_str()); // second parameter: world id string

	//Convert our std::vector to a lua table as the third parameter
	lua_newtable(ls);
	int top = lua_gettop(ls);
	for (int i = 0; i < reslist.size(); ++i)
	{
		lua_pushnumber(ls, i);
		lua_pushstring(ls, reslist.at(i).c_str());
		lua_settable(ls, top);	
	}

	int result = 1;
	if (luaCall(ls, 3, 1) != 0) // return of 0 indicates error.
	{
		console->AddMessage("\\c900 * LUA [__MAIN_BUILD] Fail");
		result = 0;
	}
	else
	{
		if (lua_isnumber(ls, -1))
		{
			result = (int)lua_tonumber(ls, -1); //get the result
		}
	}
	
	lua_pop(ls, 1); //get rid of result from stack
	
	return result;
}

/*	Calls Display() in the lua script when the map has been loaded fully and displayed onscreen */
void mapLib_CallDisplay(lua_State* ls)
{
	lua_getglobal(ls, "__MAIN_DISPLAY");
	
	if (lua_isfunction(ls, -1) && luaCall(ls, 0, 0) != 0)
	{
		console->AddMessage("\\c900 * LUA [__MAIN_DISPLAY] Fail");
	}
}

/*	Calls Destroy() in the lua script when the script is being unloaded. */
void mapLib_CallDestroy(lua_State* ls)
{
	lua_getglobal(ls, "__MAIN_DESTROY");
	
	if (lua_isfunction(ls, -1) && luaCall(ls, 0, 0) != 0)
	{
		console->AddMessage("\\c900 * LUA [__MAIN_DESTROY] Fail");	
	}
}

/*	Called in WorldLoader if the load fails, or ~Map when the map is destroyed */
void mapLib_CloseLuaState(lua_State* ls)
{
	if (!ls)
		return;
		
	mapLib_CallDestroy(ls);
	
	unregisterAllEventListeners(ls);
	unregisterAllLuaTimers(ls);
	UnregisterConvoLib(ls);

	lua_close(ls);
}

/*	Called in WorldLoader to initialize a state for us to run as the main map script */
lua_State* mapLib_OpenLuaState()
{
	lua_State* ls = luaL_newstate();
	
	// Register our functions & libraries with this new state
	luaL_openlibs( ls );  
	
	RegisterMapLib( ls );
	RegisterEventsLib( ls );
	RegisterTimersLib( ls );
	RegisterSystemLib( ls );
	RegisterEntityLib( ls );
	RegisterPlayerLib( ls );
	RegisterCameraLib( ls );
	RegisterActorLib( ls );
	RegisterConvoLib( ls );
	RegisterGameLib( ls );
	
	return ls;
}

//	.NewBasic() - Sets Game::mMap to a new initialized BasicMap class.
int map_NewBasic(lua_State* ls)
{
	console->AddMessage("\\c900DEPRECIATED Map.NewBasic()");
	return 0;
}

//	.SetSpawn(x, y) - Sets primary spawn point of the current map
int map_SetSpawn(lua_State* ls)
{
	luaCountArgs(ls, 2);
	ASSERT(game->mMap);
	
	point2d spawn;
	spawn.x = (int)lua_tonumber(ls, 1);
	spawn.y = (int)lua_tonumber(ls, 2);
	game->mMap->mSpawnPoint = spawn;
	
	return 0;
}

/*	.SetSize(w, h) - Sets map size. This determines how the camera 
		behaves (will stop at the edges of the world) (0, 0) will let
		the camera free roam.
*/
/*int map_SetSize(lua_State* ls)
{
	luaCountArgs(ls, 2);
	ASSERT(game->mMap);

	game->mMap->mWidth = (int)lua_tonumber(ls, 1);
	game->mMap->mHeight = (int)lua_tonumber(ls, 2);

	return 0;
}
*/

//	.SetColor(r, g, b) - Set background color of the map
int map_SetColor(lua_State* ls)
{
	luaCountArgs(ls, 3);
	ASSERT(game->mMap);

	game->mMap->mBackground.r = (int)lua_tonumber(ls, 1);
	game->mMap->mBackground.g = (int)lua_tonumber(ls, 2);
	game->mMap->mBackground.b = (int)lua_tonumber(ls, 3);
	game->mMap->AddCameraRectForUpdate(true);
	
	return 0;
}


//	string = .GetFlag("flag") Returns string of value. Empty string if it doesn't exist
int map_GetFlag(lua_State* ls)
{
	PRINT("map_GetFlag");
	luaCountArgs(ls, 1);

	ASSERT(game && game->mMap);

	string s = game->mMap->GetFlag( lua_tostring(ls, 1) );
	lua_pushstring(ls, s.c_str());
	
	return 1;	
}

//	.SetFlag("flag", "value")
int map_SetFlag(lua_State* ls)
{
	PRINT("map_SetFlag");
	luaCountArgs(ls, 2);

	ASSERT(game && game->mMap);

	game->mMap->SetFlag(lua_tostring(ls, 1), lua_tostring(ls, 2));

	return 0;
}

//	bool = .IsEditorMode()
int map_IsEditorMode(lua_State* ls)
{
	ASSERT(game && game->mMap);

	lua_pushboolean(ls, game->mMap->mEditorMode);
	return 1;	
}

//	.SetEditorMode(bool")
int map_SetEditorMode(lua_State* ls)
{
	luaCountArgs(ls, 1);

	ASSERT(game && game->mMap);

	game->mMap->mEditorMode = lua_toboolean(ls, 1);
	return 0;
}

//	string = .GetWorkingDir() - Returns working directory of the map (either dev/ or cache/)
int map_GetWorkingDir(lua_State* ls)
{
	string dir;

	if (game->mMap)
	{
		dir = game->mMap->mWorkingDir;
	}
	else if (game->mLoader)
	{
		if (game->mLoader->m_bTestMode)
			dir = DIR_DEV;
		else
			dir = DIR_CACHE;
	}
		
	lua_pushstring(ls, dir.c_str());
	return 1;
}

//	string = .GetID() - Returns world ID (library, wonderland, etc)
//		Will work even before NewBasic()
int map_GetID(lua_State* ls)
{
	string id;

	if (game->mMap)
		id = game->mMap->mId;
	else if (game->mLoader)
		id = game->mLoader->m_sWorldName;
		
	lua_pushstring(ls, id.c_str());
	return 1;
}

// pEnt2 = .GetNextEntityUnderMouse(pEnt, bMustBeClickable, bPlayersOnly)
int map_GetNextEntityUnderMouse(lua_State* ls)
{
	luaCountArgs(ls, 3);
	
	Entity* ent;
	
	if (lua_isnil(ls, 1))
		ent = NULL;
	else
		ent = (Entity*)lua_touserdata(ls, 1);
		
	bool mustBeClickable = lua_toboolean(ls, 2);
	bool playersOnly = lua_toboolean(ls, 3);
	
	ent = game->mMap->GetNextEntityUnderMouse(ent, mustBeClickable, playersOnly);
	
	if (ent)
		lua_pushlightuserdata(ls, ent);
	else
		lua_pushnil(ls);
		
	return 1;
}

//	bool = .IsRectBlocked(x, y, w, h)
int map_IsRectBlocked(lua_State* ls)
{
	luaCountArgs(ls, 4);
	rect r;
	r.x = (int)lua_tonumber(ls, 1);	
	r.y = (int)lua_tonumber(ls, 2);
	r.w = (int)lua_tonumber(ls, 3);
	r.h = (int)lua_tonumber(ls, 4);
	
	lua_pushboolean(ls, game->mMap->IsRectBlocked(r));
	return 1;
}

int map_GetGravity(lua_State* ls)
{
	lua_pushnumber(ls, game->mMap->GetGravity());
	return 1;	
}

int map_SetGravity(lua_State* ls)
{
	luaCountArgs(ls, 1);
	
	game->mMap->SetGravity((int)lua_tonumber(ls, 1));
	return 0;	
}

static const luaL_Reg functions[] = {
	{"NewBasic", map_NewBasic},
	{"SetSpawn", map_SetSpawn},
	{"SetColor", map_SetColor},
	{"GetFlag", map_GetFlag},
	{"SetFlag", map_SetFlag},
	{"GetWorkingDir", map_GetWorkingDir},
	{"GetID", map_GetID},
	{"IsEditorMode", map_IsEditorMode},
	{"SetEditorMode", map_SetEditorMode},
	{"GetNextEntityUnderMouse", map_GetNextEntityUnderMouse},
	{"IsRectBlocked", map_IsRectBlocked},
	{"SetGravity", map_SetGravity},
	{"GetGravity", map_GetGravity},
	{NULL, NULL}
};

void RegisterMapLib(lua_State* ls)
{
	luaL_register( ls, "Map", functions );
}


