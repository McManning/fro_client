
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
#include "../map/BasicMap.h"

/*	Calls Build() in the lua script. Return 0 on error */
int mapLib_luaCallBuild(lua_State* ls)
{
	lua_getglobal(ls, "Build");

	//if there isn't a function at the top of the stack, we failed to find it
	if (!lua_isfunction(ls, -1))
	{
		console->AddMessage("\\c900 * LUA Build() Not Found");
		return 0;
	}

	//What should be passed? lua_pushstring(ls, msg.c_str()); //first argument

	int result = 1;
	if (lua_pcall(ls, 0, 1, 0) != 0) // result = Build(), return of 0 indicates error.
	{
		console->AddMessage("\\c900 * LUA [Build] " + string(lua_tostring(ls, -1)));
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
	lua_getglobal(ls, "Display");
	
	if (lua_isfunction(ls, -1) && lua_pcall(ls, 0, 0, 0) != 0)
		console->AddMessage("\\c900 * LUA [Display] " + string(lua_tostring(ls, -1)));
}

/*	Calls Destroy() in the lua script when the script is being unloaded. */
void mapLib_CallDestroy(lua_State* ls)
{
	lua_getglobal(ls, "Destroy");
	
	if (lua_isfunction(ls, -1) && lua_pcall(ls, 0, 0, 0) != 0)
		console->AddMessage("\\c900 * LUA [Destroy] " + string(lua_tostring(ls, -1)));
}

/*	Called in WorldLoader if the load fails, or ~Map when the map is destroyed */
void mapLib_CloseLuaState(lua_State* ls)
{
	if (!ls)
		return;
		
	mapLib_CallDestroy(ls);
	
	unregisterAllEventListeners(ls);
	unregisterAllLuaTimers(ls);

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
	PRINT("map_NewBasic");
	
	ASSERT(!game->mMap);
	
	game->mMap = new BasicMap();
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
int map_SetSize(lua_State* ls)
{
	luaCountArgs(ls, 2);
	ASSERT(game->mMap);

	game->mMap->mWidth = (int)lua_tonumber(ls, 1);
	game->mMap->mHeight = (int)lua_tonumber(ls, 2);

	return 0;
}

//	.SetColor(r, g, b) - Set background color of the map
int map_SetColor(lua_State* ls)
{
	luaCountArgs(ls, 3);
	ASSERT(game->mMap);

	game->mMap->mBackground.r = (int)lua_tonumber(ls, 1);
	game->mMap->mBackground.g = (int)lua_tonumber(ls, 2);
	game->mMap->mBackground.b = (int)lua_tonumber(ls, 3);
	
	return 0;
}

static const luaL_Reg functions[] = {
	{"NewBasic", map_NewBasic},
	{"SetSpawn", map_SetSpawn},
	{"SetSize", map_SetSize},
	{"SetColor", map_SetColor},
	{NULL, NULL}
};

void RegisterMapLib(lua_State* ls)
{
	luaL_register( ls, "Map", functions );
}


