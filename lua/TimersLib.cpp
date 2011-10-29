
/*
 * Copyright (c) 2011 Chase McManning
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights 
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 */


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
		luaReference = LUA_NOREF;
	};
	~LuaTimer()
	{
		luaL_unref(luaState, LUA_REGISTRYINDEX, luaReference);
	};

	int DoFunction(timer* t, uLong ms);

	lua_State* luaState; //lua_state containing our target lua function
	string luaFunction; //function that will be called during this message dispatch.
	
	int luaReference; //Unique id of the attached lua object
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

	//first argument, handle
	lua_pushnumber(luaState, t->handle); 

	//second argument, interval
	lua_pushnumber(luaState, t->interval); 

	//third argument, our stored lua object
	lua_rawgeti(luaState, LUA_REGISTRYINDEX, luaReference);

	int result = 0;
	if (luaCall(luaState, 3, 1) != 0)
	{
		console->AddMessage("\\c900 * LUATIMER [" + luaFunction + "] Fail");
        result = 0;
	}
	else
	{
		if (lua_isnumber(luaState, -1))
		{
			result = (int)lua_tonumber(luaState, -1); //get the result
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

//	.Add(interval, "lua_callback", userdata); Userdata can be any lua object. 
//		Returns the handle of the new timer, or 0 if something went wrong.
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
	
	//if we supplied a lua object, create a reference to it
	if (numArgs > 2)
	{
		lua_pushvalue(ls, 3); //copy the value at index to the top of the stack
		lt->luaReference = luaL_ref(ls, LUA_REGISTRYINDEX); //create a reference to the stack top and pop
	}	
		
	timer* t = timers->Add(lt->luaFunction, interval, false, timer_luaActivate, timer_luaDestroy, lt);
	
	uLong handle = INVALID_TIMER_HANDLE;
	if (t)
		handle = t->handle;
		
	lua_pushnumber(ls, handle);
	return 1;
}

// bool = .RemoveHandle(handle) - Unregister a timer with the matching handle number. Returns true if successful, false otherwise.
int timers_RemoveHandle(lua_State* ls)
{
	PRINT("timers_RemoveHandle");
	luaCountArgs(ls, 1);
	
	uLong handle = (uLong)lua_tonumber(ls, 1);
	
	bool result = timers->RemoveByHandle(handle);
	
	lua_pushboolean(ls, result);
	return 1;
}

// .Remove("timer_callback");
// Only Unregisters events matching the matching lua_State, so it won't unregister event listeners outside this script.
int timers_Remove(lua_State* ls)
{
	PRINT("timers_Remove");
	luaCountArgs(ls, 1);
	
	int numArgs = lua_gettop( ls );

	string func = lua_tostring(ls, 1);

	LuaTimer* lt;
	bool valid;
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
	{"RemoveHandle", timers_RemoveHandle},
	{"RemoveAll", timers_RemoveAll},
	{NULL, NULL}
};

void RegisterTimersLib(lua_State* ls)
{
	luaL_register( ls, "Timers", functions );
}



