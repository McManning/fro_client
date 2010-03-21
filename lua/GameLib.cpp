
#include <lua.hpp>
#include "GameLib.h"
#include "LuaCommon.h"
#include "../game/GameManager.h"
#include "../map/Map.h"

// .Print("Message")
int game_Print(lua_State* ls)
{
	PRINT("game_Print");
	luaCountArgs(ls, 1);

	game->mChat->AddFormattedMessage( lua_tostring(ls, 1) );
	
	return 0;
}

//TODO: MOVE!

//	string = .GetMapFlag("flag") Returns string of value. Empty string if it doesn't exist
int game_GetMapFlag(lua_State* ls)
{
	PRINT("game_GetMapFlag");
	luaCountArgs(ls, 1);

	ASSERT(game && game->mMap);

	string s = game->mMap->GetFlag( lua_tostring(ls, 1) );
	lua_pushstring(ls, s.c_str());
	
	return 1;	
}

//	.SetMapFlag("flag", "value")
int game_SetMapFlag(lua_State* ls)
{
	PRINT("game_SetMapFlag");
	luaCountArgs(ls, 2);

	ASSERT(game && game->mMap);

	game->mMap->SetFlag(lua_tostring(ls, 1), lua_tostring(ls, 2));

	return 0;
}


static const luaL_Reg functions[] = {
	{"Print", game_Print},
	{"GetMapFlag", game_GetMapFlag},
	{"SetMapFlag", game_SetMapFlag},
	{NULL, NULL}
};

void RegisterGameLib(lua_State* ls)
{
	luaL_register( ls, "Game", functions );
}


