
#include <lua.hpp>
#include "LuaCommon.h"

bool luaCountArgs(lua_State* ls, int desired)
{
	int numArgs = lua_gettop( ls );
	if (numArgs < desired)
	{
		string func = "???";
		
		lua_Debug dbg;
		if (lua_getstack(ls, 0, &dbg))
		{
			lua_getinfo(ls, "nl", &dbg);
			if (dbg.name == NULL)
				func = "(NULL)";
			else
				func = dbg.name;
		}

		string error = func + ": Invalid Parameter Count (Minimum: " + its(desired) + ")";

		lua_pushstring( ls, error.c_str() );
		lua_error( ls ); //This function does a longjmp and never returns.
		return false;
	}
	return true;
}

int luaError(lua_State* ls, string func, string msg)
{
/*	string func = "???";
	int line = -1;
	
	lua_Debug dbg;
	if (lua_getstack(ls, 0, &dbg))
	{
		func = ":3";
		lua_getinfo(ls, "nls", &dbg);
		if (dbg.name == NULL)
			func = "(NULL)";
		else
			func = dbg.name;
		line = dbg.currentline;
	}

	console->AddFormattedMessage("\\c900 * LUA [" + func + ":" + its(line) + ":" + dbg.short_src + "] " + msg);
	*/

	string error = "[" + func + "] " + msg;

	lua_pushstring( ls, error.c_str() );
	lua_error( ls ); //This function does a longjmp and never returns.
	
	return 0;
}
