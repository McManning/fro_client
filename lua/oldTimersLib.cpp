
#include <lua.hpp>
#include "TimersLib.h"
#include "LuaCommon.h"

/*
--------------------------- TIMER RELATED -----------------------------

-- Return interval until next time it's called. 0 to destroy the timer.
function timer_callback(userdata, interval, tick)

	return interval;
end

Timers.Add(interval, "timer_callback", userdata);

-- parameter 2 being optional. If set, will only remove events matching with both properties. 
-- Also only unregisters events matching the matching lua_State, so it won't unregister event listeners outside this script.
Timers.Remove("timer_callback", userdata);

Timers.RemoveAll(); -- Removes all functions with this lua_State

The problem with the above is... how do we store userdata?
I want it to be ANYTHING passed from lua, whether a table, string, cptr, lua function, etc.
But how do we store that information in C?
If there isn't an easy way... from within C we can do this:
	detect what type of value was passed into userdata in via Timers.Add("func", userdata, ...) via C and lua_isstring, lua_isnumber, etc.
	From there, we have the same kind of system like LuaMessageListener, stores it in appropriate place, then converts, etc.
	However, we wouldn't be supporting functions, or tables, but... meh! Do we really need to? No. 
	

// .Add(interval, "luafunc", userdata)  - Userdata can be a cptr, string, or integer.
*/



		
//Stored as timer userdata for lua-callback timers
class LuaTimer
{
  public:
	LuaTimer()
	{
		luaState = NULL;
	};

	int DoFunction(timer* t, uLong ms);

	lua_State* luaState; //lua_state containing our target lua function
	string luaFunction; //function that will be called during this message dispatch.
	
	luaDataType type;
	void* u;
	int i;
	string s;
};

//Returns the new interval after the function is called. If it returns 0, destroy the timer.
int LuaTimer::DoFunction(timer* t, uLong ms)
{
	if (!luaState) return 0;

	lua_getglobal(luaState, luaFunction.c_str()); //get function name

	//if there isn't a function at the top of the stack, we failed to find it
	if (!lua_isfunction(luaState, -1))
	{
		//cannot be luaError because we need to return!
		WARNING("Lua Function " + luaFunction + " not found during firing of timer");
		return 0;
	}

	lua_pushnumber(luaState, t->interval); //first argument

	switch (type) //second argument, depends on stored data type
	{
		case STRING:
			lua_pushstring(luaState, s.c_str());
			break;
		case INTEGER:
			lua_pushnumber(luaState, i);
			break;
		case USERDATA:
			lua_pushlightuserdata(luaState, u);
			break;
		default: 
			lua_pushnumber(luaState, 0);
			break;
	}

	lua_pushnumber(luaState, ms); //third argument

	int result;
	if (lua_pcall(luaState, 3, 1, 0) != 0)
	{
		console->AddFormattedMessage("\\c900 * LUATIMER [" + luaFunction + "] " + string(lua_tostring(luaState, -1)));
        result = 0; //luaError( luaState, luaFunction, lua_tostring(luaState, -1) );
	}
	else
	{
		if (lua_isnumber(luaState, -1))
		{
			result = lua_isnumber(luaState, -1); //get the result
		}
	}

	lua_pop(luaState, 1); //get rid of result from stack

	t->interval = result;
	
	return result;
}

uShort timer_luaActivate(timer* t, uLong ms)
{
	LuaTimer* lt = (LuaTimer*)t->userData;

	if (!lt || lt->DoFunction(t, ms) == 0)
		return TIMER_DESTROY;

	return TIMER_CONTINUE;
}

void timer_luaDestroy(timer* t)
{
	LuaTimer* lt = (LuaTimer*)t->userData;
	SAFEDELETE(lt);
	
	t->userData = NULL;
}

//.Add(interval, "lua_callback", userdata); Userdata can be string, integer, or cptr. Optional.
int timers_Add(lua_State* ls)
{
	PRINT("timers_Add");
	luaCountArgs(ls, 2);

	int numArgs = lua_gettop( ls );

	uLong interval = (uLong)lua_tonumber(ls, 1);
	
	LuaTimer* lt;
	lt = new LuaTimer();
	lt->luaFunction = lua_tostring(ls, 2);
	lt->luaState = ls;
	
	if (numArgs > 2)
	{
		//Figure out our userdata type and store.
		if (lua_islightuserdata(ls, 3))
		{
			lt->type = LUATYPE_USERDATA;
			lt->u = lua_touserdata(ls, 3);
		}
		else if (lua_isnumber(ls, 3))
		{
			lt->type = LUATYPE_INTEGER;
			lt->i = (int)lua_tonumber(ls, 3);
		}
		else if (lua_isstring(ls, 3))
		{
			lt->type = LUATYPE_STRING;
			lt->s = lua_tostring(ls, 3);
		}
		else 
		{
			WARNING("[timers_Add] Invalid userdata for " + lt->luaFunction);
			//default
			lt->type = LUATYPE_INTEGER;
			lt->i = 0;
		}
	}
	else
	{
		lt->type = LUATYPE_INTEGER;
		lt->i = 0;	
	}
	
	timer* t = timers->Add(lt->luaFunction, interval, false, timer_luaActivate, timer_luaDestroy, lt);
	
	return 0;
}

//Timers.Remove("timer_callback", userdata);
// parameter 2 being optional. If set, will only remove events matching with both properties. 
// Also only unregisters events matching the matching lua_State, so it won't unregister event listeners outside this script.
int timers_Remove(lua_State* ls)
{
	PRINT("timers_Remove");
	luaCountArgs(ls, 1);
	
	int numArgs = lua_gettop( ls );

	string func = lua_tostring(ls, 1);

	LuaTimer* lt;
	bool valid = true;
	for (int i = 0; i < timers->mTimers.size(); i++)
	{
		//if it's not a lua timer
		if ( !timers->mTimers.at(i) || timers->mTimers.at(i)->onActivate != timer_luaActivate )
			continue;

		lt = (LuaTimer*)timers->mTimers.at(i)->userData;
		
		//If it wasn't created by this lua_state
		if ( !lt || lt->luaState == ls )
			continue;

		//If the function doesn't match
		if ( lt->luaFunction != func )
			continue;

		if (numArgs > 1) //check for userdata match
		{
			if (lua_islightuserdata(ls, 2))
			{
				if ( lt->type != LUATYPE_USERDATA || lt->u != lua_touserdata(ls, 2) )
					valid = false;
			}
			else if (lua_isnumber(ls, 2))
			{
				if ( lt->type != LUATYPE_INTEGER || lt->i != lua_tonumber(ls, 2) )
					valid = false;
			}
			else if (lua_isstring(ls, 2))
			{
				if ( lt->type != LUATYPE_STRING || lt->s != lua_tostring(ls, 2) )
					valid = false;
			}
			else //consider invalid
			{
				valid = false;
			}
		}
		
		if (valid)
			timers->Remove( timers->mTimers.at(i) );
	}
	
	return 0;
}

// .RemoveAll() - Deletes all timers referencing our lua_State
int timers_RemoveAll(lua_State* ls)
{
	PRINT("timers_RemoveAll");

	unregisterAllLuaTimers(ls);
	return 0;
}

void unregisterAllLuaTimers(lua_State* ls)
{
	LuaTimer* lt;
	for (int i = 0; i < timers->mTimers.size(); i++)
	{
		//if it's a lua listener, convert and check userdata
		if (timers->mTimers.at(i) && timers->mTimers.at(i)->onActivate == timer_luaActivate)
		{
			lt = (LuaTimer*)timers->mTimers.at(i)->userData;
			
			//if this listener exists and was created by this lua_state...
			if (lt && lt->luaState == ls)
			{
				timers->Remove( timers->mTimers.at(i) );
			}
		}
	}
}

static const luaL_Reg functions[] = {
	{"Add", timers_Add},
	{"Remove", timers_Remove},
	{"RemoveAll", timers_RemoveAll},
	{NULL, NULL}
};

void RegisterTimersLib(lua_State* ls)
{
	luaL_register( ls, "Timers", functions );
}



