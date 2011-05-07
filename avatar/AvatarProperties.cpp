
#include <lua.hpp>
#include "AvatarProperties.h"

// Pulls data from the table at the specific index. -1 for stack top
bool AvatarProperties::FromLuaTable(lua_State* ls, int virtualIndex)
{
	string key;
	
	if (virtualIndex < 0) // conversion from virtual to real for lua_next
		virtualIndex = lua_gettop(ls) + virtualIndex + 1;
		
	if (!lua_istable(ls, virtualIndex))
		return false;

	lua_pushnil(ls);
	while (lua_next(ls, virtualIndex) != 0)
	{
		if (lua_isstring(ls, -2))
		{
			key = lua_tostring(ls, -2);
			if (key == "url")
				url = lua_tostring(ls, -1);
			else if (key == "w")
				w = (int)lua_tonumber(ls, -1);
			else if (key == "h")
				h = (int)lua_tonumber(ls, -1);
			else if (key == "pass")
				pass = lua_tostring(ls, -1);
			else if (key == "delay")
				delay = (int)lua_tonumber(ls, -1);
			else if (key == "flags")
				flags = (int)lua_tonumber(ls, -1);
		}
		lua_pop(ls, 1);
	}
	
	return true;
}

// Adds a new table to the top of the lua_State's stack.
bool AvatarProperties::ToLuaTable(lua_State* ls)
{
	lua_newtable(ls);
	int top = lua_gettop(ls);

	lua_pushstring(ls, "url");
		lua_pushstring(ls, url.c_str());
		lua_settable(ls, top);
	lua_pushstring(ls, "pass");
		lua_pushstring(ls, pass.c_str());
		lua_settable(ls, top);
	lua_pushstring(ls, "w");
		lua_pushnumber(ls, w);
		lua_settable(ls, top);
	lua_pushstring(ls, "h");
		lua_pushnumber(ls, h);
		lua_settable(ls, top);	
	lua_pushstring(ls, "delay");
		lua_pushnumber(ls, delay);
		lua_settable(ls, top);
	lua_pushstring(ls, "flags");
		lua_pushnumber(ls, flags);
		lua_settable(ls, top);
		
	return true;
}
