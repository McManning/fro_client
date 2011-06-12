
#include <lua.hpp>
#include <stdarg.h>
#include "LuaCommon.h"

/**
	Simple wrapper over lua_pcall. Allows us to send in a list of parameters, and specify number of return values.
	Currently only supports char*, int, and double as input parameters. 
	
	@param func Global name of the function we want to call
	@param returncount Number of return values expected on the stack
	@param params Format string of the parameters we want to pass to the lua function
	
	@return result from lua_pcall (Nonzero if an error occured)
*/
/*
	Example:
		result = luaPCall(myLuaState, "HelloWorld", 1, "ssif", "Some string", "Another string", 14, 15.2);
*/
int luaCallWith(lua_State* ls, const char* func, int returncount, char* params, ...)
{
	int total = 0;
    va_list args;
    va_start(args, params);
	
	while (*params != '\0')
	{
		if (*params == 's')
		{
			lua_pushstring(ls, va_arg(args, char*));
			++total;
		}
		else if (*params == 'd' || *params == 'i')
		{
			lua_pushnumber(ls, va_arg(args, int));
			++total;
		}
		else if (*params == 'f')
		{
			lua_pushnumber(ls, va_arg(args, double));
			++total;
		}
		
		++params;
	}
	
	va_end(args);
	
	return lua_pcall(ls, total, returncount, 0);
}

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
		luaError(ls, "", error);
		
		return false;
	}
	return true;
}

static int luaTraceback(lua_State *L) 
{
	lua_getfield(L, LUA_GLOBALSINDEX, "debug");
	if (!lua_istable(L, -1)) {
	lua_pop(L, 1);
	return 1;
	}
	lua_getfield(L, -1, "traceback");
	if (!lua_isfunction(L, -1)) {
	lua_pop(L, 2);
	return 1;
	}
	// lua_pushvalue(L, 1);  /* pass error message */
	// lua_pushinteger(L, 2);  /* skip this function and traceback */
	lua_call(L, 0, 1);  /* call debug.traceback */
	
	if (lua_isstring(L, -1))
	{
		string s = lua_tostring(L, -1);
		console->AddMessage("\\c099" + s);
	}
	lua_pop(L, 1);

	return 1;
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

	console->AddMessage("\\c900 * LUA [" + func + ":" + its(line) + ":" + dbg.short_src + "] " + msg);
	*/

	string error = "[" + func + "] " + msg;

	console->AddMessage("\\c900Internal Lua Error: " + error);

	luaStackdump(ls);
	luaTraceback(ls);

	lua_pushstring( ls, error.c_str() );
	lua_error( ls ); //This function does a longjmp and never returns.
	
	return 0;
}

void luaStackdump(lua_State* l)
{
    int i;
    int top = lua_gettop(l);

	string s = "\\c990stack dump\n";

    for (i = 1; i <= top; i++)
    {  /* repeat for each level */
        int t = lua_type(l, i);
        switch (t) {
            case LUA_TSTRING:  /* strings */
                s += "\t[" + its(i) + "] string: " + string(lua_tostring(l, i)) + "\n";
                break;
            case LUA_TBOOLEAN:  /* booleans */
                s += "\t[" + its(i) + "] boolean: " + string(lua_toboolean(l, i) ? "true\n" : "false\n");
                break;
            case LUA_TNUMBER:  /* numbers */
                s += "\t[" + its(i) + "] number: " + string(lua_tostring(l, i)) + "\n";
                break;
            default:  /* other values */
                s += "\t[" + its(i) + "] " + string(lua_typename(l, t)) + "\n";
                break;
        }
    }
    
    console->AddMessage(s);
}

int luaCall(lua_State* ls, int a, int b)
{
	int result = lua_pcall(ls, a, b, 0);
	
	if (result != 0) //something went wrong, dump it
	{
		console->AddMessage("\\c900Caught Lua Error: " + string(lua_tostring(ls, -1)));

		luaStackdump(ls);
		luaTraceback(ls);
	}
	
	return result;	
}



