
#ifndef _LUA_MAPLIB_H_
#define _LUA_MAPLIB_H_

#include "../core/Core.h"

/*	Called in WorldLoader if the load fails, or ~Map when the map is destroyed */
void mapLib_CloseLuaState(lua_State*);

/*	Called in WorldLoader to initialize a state for us to run as the main map script */
lua_State* mapLib_OpenLuaState();

/*	Calls Build() in the lua script. Return 0 on error */
int mapLib_luaCallBuildWorld(lua_State*, bool, string&, std::vector<string>&);

/*	Calls Display() in the lua script when the map has been loaded fully and displayed onscreen */
void mapLib_CallDisplay(lua_State*);

void RegisterMapLib(lua_State*);

#endif //_LUA_MAPLIB_H_
