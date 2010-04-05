
#include <lua.hpp>
#include "EventsLib.h"
#include "LuaCommon.h"
#include "../core/MessageManager.h"
#include "../core/MessageData.h"

/*
------------------------- LUA CODE EXAMPLE -------------------------------

-- Return 0 to destroy this event listener. 1 to keep it running.
function event_callback(eventId, dataTable)
	
	--let us assume this data table has a key 'entity' with a value of a Entity* cptr

	ent = dataTable["entity"];
	Entity.Say(ent, "Hello from event listener " .. eventId);
	
	return 1;
end

Events.Register("SOME_EVENT", "event_callback");

-- parameter 2 being optional. If set, will only remove events matching with both properties. 
-- Also only unregisters events matching the matching lua_State, 
-- so it won't unregister event listeners outside this script.
Events.Unregister("SOME_EVENT", "event_callback"); 

Events.UnregisterAll(); -- Removes all event listeners with this lua_State
*/

class LuaMessageListener
{
  public:
	LuaMessageListener()
	{
		luaState = NULL;
		luaReference = LUA_NOREF;
	};
	
	~LuaMessageListener()
	{
		luaL_unref(luaState, LUA_REGISTRYINDEX, luaReference);
	};
	
	// Call the lua function within the state, converting and passing MessageData into it.
	//if it returns -1, delete the listener from the manager.
	int DoFunction(MessageListener*, MessageData&);

	lua_State* luaState; //lua_state containing our target lua function
	string luaFunction; //function that will be called during this message dispatch.
	
	int luaReference; //Unique id of the attached lua object
};

int LuaMessageListener::DoFunction(MessageListener* ml, MessageData& data)
{
	if (!luaState) return 0;

PRINT("LuaMessageListener::DoFunction get function");

	lua_getglobal(luaState, luaFunction.c_str()); //get function name

	//if there isn't a function at the top of the stack, we failed to find it
	if (!lua_isfunction(luaState, -1))
	{
		WARNING("Lua Function " + luaFunction + " not found during firing of event " + data.mId);
		return 0;	
	}

PRINT("LuaMessageListener::DoFunction push args");

	//first argument, listener handle
	lua_pushnumber(luaState, ml->handle);

	//second argument, event id
	lua_pushstring(luaState, data.mId.c_str()); 

	//Convert our std::map to a lua table as the third parameter
	lua_newtable(luaState);
	int top = lua_gettop(luaState);

	for (std::map<string, MessageData::messageItem>::iterator it = data.mData.begin(); it != data.mData.end(); ++it) 
	{
		const char* key = it->first.c_str();
		lua_pushstring(luaState, key);
		
		//Add a data type that corrosponds to our value's type. 
		MessageData::messageItem& item = it->second;
		switch (item.type)
		{
			case STRING:
				lua_pushstring(luaState, item.s.c_str());
				break;
			case INTEGER:
				lua_pushnumber(luaState, item.i);
				break;
			case USERDATA:
				lua_pushlightuserdata(luaState, item.u);
				break;
			default: break;
		}
		lua_settable(luaState, top);
	}
	
	//fourth argument, our stored lua object
	lua_rawgeti(luaState, LUA_REGISTRYINDEX, luaReference);

	int result;

PRINT("LuaMessageListener::DoFunction lua_pcall");

	if (lua_pcall(luaState, 4, 1, 0) != 0)
	{
		console->AddFormattedMessage("\\c900 * LUAEVENT [" + luaFunction + "] " + string(lua_tostring(luaState, -1)));
		//result = luaError( luaState, luaFunction, lua_tostring(luaState, -1) )
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
	
PRINT("LuaMessageListener::DoFunction end");

	return result;
}

void listener_luaActivate(MessageListener* ml, MessageData& data, void* sender)
{
	LuaMessageListener* luaListener = (LuaMessageListener*)ml->userdata;
	
	//If bad userdata, or the function returns 0, delete listener.
	if (!luaListener || luaListener->DoFunction(ml, data) == -1)
	{
		messenger.RemoveListener(ml);
	}
}

void listener_luaDestroy(MessageListener* ml)
{
	LuaMessageListener* luaListener = (LuaMessageListener*)ml->userdata;
	SAFEDELETE(luaListener);
	
	ml->userdata = NULL;
}

void unregisterAllEventListeners(lua_State* ls)
{
	LuaMessageListener* luaListener;
	for (int i = 0; i < messenger.mListeners.size(); i++)
	{
		//if it's a lua listener, convert and check userdata
		if (messenger.mListeners.at(i) && messenger.mListeners.at(i)->onActivate == listener_luaActivate)
		{
			luaListener = (LuaMessageListener*)messenger.mListeners.at(i)->userdata;
			
			//if this listener exists and was created by this lua_state...
			if (luaListener && luaListener->luaState == ls)
			{
				messenger.RemoveListener( messenger.mListeners.at(i) );
			}
		}
	}
}

// .Register("id", "luafunc", userdata<nil>) - Returns the unique handle of the newly created listener
int events_Register(lua_State* ls)
{
	PRINT("events_Register");
	luaCountArgs(ls, 2);

	int numArgs = lua_gettop( ls );
	
	if (!lua_isstring(ls, 1) || !lua_isstring(ls, 2))
		return luaError(ls, "Events.Register", "Bad Params");

	string id = lua_tostring(ls, 1);
	string func = lua_tostring(ls, 2);

	LuaMessageListener* luaListener = new LuaMessageListener();
	luaListener->luaState = ls;
	luaListener->luaFunction = func;
	
	if (numArgs > 2)
	{
		lua_pushvalue(ls, 3); //copy the value at index to the top of the stack
		luaListener->luaReference = luaL_ref(ls, LUA_REGISTRYINDEX); //create a reference to the stack top and pop
	}
	
	MessageListener* ml = messenger.AddListener(id, listener_luaActivate, listener_luaDestroy, luaListener);
	
	lua_pushnumber(ls, ml->handle); //return our handle
	return 1;
}

//	bool = .UnregisterHandle(handle) - Unregisters the event with the handle. Returns true on success, false otherwise.
int events_UnregisterHandle(lua_State* ls)
{
	PRINT("events_UnregisterHandle");
	luaCountArgs(ls, 1);
	
	uLong handle = (uLong)lua_tonumber(ls, 1);
	
	bool result = messenger.RemoveListenerByHandle(handle);
	
	lua_pushboolean(ls, result);
	return 1;
}

// .Unregister("id", "luafunc") - luafunc is optional, if supplied, will unregister those listeners matching both id and luafunc.
// Note: This will only unregister matching data from THIS LUA_STATE. Will not damage any other similar outside listeners.
int events_Unregister(lua_State* ls)
{
	PRINT("events_Unregister");
	luaCountArgs(ls, 1);
	
	int numArgs = lua_gettop( ls );

	string id = lua_tostring(ls, 1);
	string func;
	if (numArgs > 1)
		func = lua_tostring(ls, 2);

	LuaMessageListener* luaListener;
	for (int i = 0; i < messenger.mListeners.size(); i++)
	{
		//if it's a lua listener, convert and check userdata
		if (messenger.mListeners.at(i) && messenger.mListeners.at(i)->onActivate == listener_luaActivate)
		{
			luaListener = (LuaMessageListener*)messenger.mListeners.at(i)->userdata;
			
			//if this listener exists and was created by this lua_state...
			if (luaListener && luaListener->luaState == ls)
			{
				//if we have a matching event ID, and a matching lua function name (if supplied), erase it!
				if ( messenger.mListeners.at(i)->id == id 
						&& (func.empty() || luaListener->luaFunction == func) )
				{
					messenger.RemoveListener( messenger.mListeners.at(i) );
				}
			}
		}
	}
	
	return 0;
}

// .UnregisterAll() - Deletes all message listeners referencing our lua_State
int events_UnregisterAll(lua_State* ls)
{
	PRINT("events_UnregisterAll");

	unregisterAllEventListeners(ls);
	return 0;
}

static const luaL_Reg events_funcs[] = {
	{"Register", events_Register},
	{"Unregister", events_Unregister},
	{"UnregisterHandle", events_UnregisterHandle},
	{"UnregisterAll", events_UnregisterAll},
	{NULL, NULL}
};

void RegisterEventsLib(lua_State* ls)
{
	luaL_register( ls, "Events", events_funcs );
}
