
#include <lua.hpp>
#include "SystemLib.h"
#include "LuaCommon.h"
#include "../core/widgets/MessagePopup.h"
#include "../core/widgets/OpenUrl.h"
#include "../core/io/FileIO.h"

// .Print("Message")
int system_Print(lua_State* ls)
{
	PRINT("system_Print");
	luaCountArgs(ls, 1);

	console->AddFormattedMessage( lua_tostring(ls, 1) );
	
	return 0;
}

// .Fatal("Message")
int system_Fatal(lua_State* ls)
{
	PRINT("system_Fatal");
	luaCountArgs(ls, 1);

	FATAL( lua_tostring(ls, 1) );
	
	return 0;
}

// .Alert("id", "Title", "Message")
int system_Alert(lua_State* ls)
{
	PRINT("system_Alert");
	luaCountArgs(ls, 2);

	FATAL( lua_tostring(ls, 1) );
	
	new MessagePopup(lua_tostring(ls, 1), lua_tostring(ls, 2), lua_tostring(ls, 3), false);
	
	return 0;
}

// .MessageDialog("id", "Title", "Message")
int system_MessageDialog(lua_State* ls)
{
	PRINT("system_MessageDialog");
	luaCountArgs(ls, 2);

	FATAL( lua_tostring(ls, 1) );
	
	new MessagePopup(lua_tostring(ls, 1), lua_tostring(ls, 2), lua_tostring(ls, 3), true);
	
	return 0;
}

// .Wildmatch("pattern", "message") - Returns 1 if the message matches the pattern, 0 otherwise
int system_Wildmatch(lua_State* ls)
{
	PRINT("system_Wildmatch");
	luaCountArgs(ls, 2);

	lua_pushnumber( ls, wildmatch(lua_tostring(ls, 1), lua_tostring(ls, 2)) );
	
	return 1;
}

// .OpenUrl("url") - Open an OpenUrl request
int system_OpenUrl(lua_State* ls)
{
	PRINT("system_OpenUrl");
	luaCountArgs(ls, 1);

	new OpenUrl( lua_tostring(ls, 1) );
	
	return 0;
}

// .GetTheta(x, y, x2, y2) - Returns theta (in degrees), using (x,y) as origin.
int system_GetTheta(lua_State* ls)
{
	PRINT("system_GetTheta");
	luaCountArgs(ls, 4);
	
	double dx = lua_tonumber(ls, 3) - lua_tonumber(ls, 1);
	double dy = lua_tonumber(ls, 4) - lua_tonumber(ls, 2);
	double theta = atan2( dy, dx );
	
	lua_pushnumber( ls, theta );
	
	return 1;
}

// .GetDistance(x, y, x2, y2) - Returns distance between (x,y) and (x2,y2)
int system_GetDistance(lua_State* ls)
{
	PRINT("system_GetDistance");
	luaCountArgs(ls, 4);
	
	double distance = getDistance( point2d((sShort)lua_tonumber(ls, 1), (sShort)lua_tonumber(ls, 2)), 
									point2d((sShort)lua_tonumber(ls, 3), (sShort)lua_tonumber(ls, 4)) );
	lua_pushnumber( ls, distance );
	
	return 1;
}

// x, y = .OffsetByTheta(x, y, theta, distance) - Returns a new point, offset from the original
int system_OffsetByTheta(lua_State* ls)
{
	PRINT("system_OffsetByTheta");
	luaCountArgs(ls, 4);

	double theta = lua_tonumber(ls, 3);
	double distance = lua_tonumber(ls, 4);
	
	lua_pushnumber( ls, lua_tonumber(ls, 1) + distance * cos(theta * M_PI / 180) ); // x
	lua_pushnumber( ls, lua_tonumber(ls, 2) + distance * sin(theta * M_PI / 180) ); // y
	
	return 2;
}

// .StringToNumber("string") - Hash the string into an integer, usable as a random number seed
int system_StringToNumber(lua_State* ls)
{
	PRINT("system_StringToNumber");
	luaCountArgs(ls, 1);
	
	if (!lua_isstring(ls, 1))
		return luaError(ls, "System.StringToNumber", "Invalid Param");
	
	int n = 0;
	string s = lua_tostring(ls, 1);
	
	for (int i = 0; i < s.size(); i++)
		n += s.at(i);
	
	n *= s.size();
	
	lua_pushnumber(ls, n);
	return 1;
}

//	.GenerateFilename("key") - Returns a random named cache file, generated from the key value
int system_GenerateFilename(lua_State* ls)
{
	PRINT("system_GenerateFilename");
	luaCountArgs(ls, 1);
	
	string key = lua_tostring(ls, 1);
	
	string file = DIR_CACHE;
	file += "lua." + md5(key.c_str(), key.length());
	
	lua_pushstring(ls, file.c_str());
	return 1;
}

static const luaL_Reg functions[] = {
	{"Print", system_Print},
	{"Fatal", system_Fatal},
	{"MessageDialog", system_MessageDialog},
	{"Alert", system_Alert},
	{"Wildmatch", system_Wildmatch},
	{"OpenUrl", system_OpenUrl},
	{"GetTheta", system_GetTheta},
	{"GetDistance", system_GetDistance},
	{"OffsetByTheta", system_OffsetByTheta},
	{"StringToNumber", system_StringToNumber},
	{"GenerateFilename", system_GenerateFilename},
	{NULL, NULL}
};

void RegisterSystemLib(lua_State* ls)
{
	luaL_register( ls, "System", functions );
}
