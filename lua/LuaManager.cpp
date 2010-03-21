
#include <lua.hpp>
#include "LuaManager.h"
#include "../game/GameManager.h"

#include "EventsLib.h"
#include "TimersLib.h"
#include "SystemLib.h"
#include "EntityLib.h"
#include "PlayerLib.h"
#include "CameraLib.h"
#include "ActorLib.h"
#include "ConvoLib.h"
#include "GameLib.h"

LuaManager::LuaManager()
{
	mState = luaL_newstate();
	
	//Register our functions & libraries with this new state
	luaL_openlibs( mState );  
	
	RegisterEventsLib( mState );
	RegisterTimersLib( mState );
	RegisterSystemLib( mState );
	RegisterEntityLib( mState );
	RegisterPlayerLib( mState );
	RegisterCameraLib( mState );
	RegisterActorLib( mState );
	RegisterConvoLib( mState );
	RegisterGameLib( mState );
}

LuaManager::~LuaManager()
{
	PRINT("LuaManager::~LuaManager 1");
	OnUnload();
	
	PRINT("LuaManager::~LuaManager 2");
	unregisterAllEventListeners(mState);
	
	PRINT("LuaManager::~LuaManager 3");
	unregisterAllLuaTimers(mState);
	
	PRINT("LuaManager::~LuaManager 4");
	lua_close(mState);
	
	PRINT("LuaManager::~LuaManager end");
}

bool LuaManager::Load(string file)
{
	mFilename = file;
	if ( luaL_dofile( mState, file.c_str() ) != 0 )
	{
		mError = lua_tostring( mState, -1 );
		return false;
	}
	
	return true;
}

int LuaManager::OnLoad()
{
	lua_getglobal(mState, "OnLoad"); //get function name

	//if there isn't a function at the top of the stack, we failed to find it
	if (!lua_isfunction(mState, -1))
	{
		mError = "OnLoad() not found";
		return -1;	
	}

	//What should be passed? lua_pushstring(mState, msg.c_str()); //first argument

	int result = 0;
	if (lua_pcall(mState, 0, 1, 0) != 0)
	{
		console->AddFormattedMessage("\\c900 * LUA [OnLoad] " + string(lua_tostring(mState, -1)));
		//result = luaError( luaState, luaFunction, lua_tostring(luaState, -1) )
        result = -1;
	}
	else
	{
		if (lua_isnumber(mState, -1))
		{
			result = (int)lua_tonumber(mState, -1); //get the result
		}
	}
		
	//lua_call(mState, 0, 1); //lua_state, # params passed, # params to return
	//int result = (int)lua_tointeger(mState, -1); //get the result

	lua_pop(mState, 1); //get rid of result from stack
	
	return result;
}

int LuaManager::OnUnload()
{
	lua_getglobal(mState, "OnUnload"); //get function name

	//if there isn't a function at the top of the stack, we failed to find it
	if (!lua_isfunction(mState, -1))
	{
		mError = "OnUnload() not found";
		return -1;	
	}

	int result = 0;
	if (lua_pcall(mState, 0, 1, 0) != 0)
	{
		console->AddFormattedMessage("\\c900 * LUA [OnUnload] " + string(lua_tostring(mState, -1)));
		//result = luaError( luaState, luaFunction, lua_tostring(luaState, -1) )
        result = -1;
	}
	else
	{
		if (lua_isnumber(mState, -1))
		{
			result = (int)lua_tonumber(mState, -1); //get the result
		}
	}

	lua_pop(mState, 1); //get rid of result from stack
	
	return result;
}

int LuaManager::ReadXml(XmlFile* xf, TiXmlElement* e, bool online)
{
	string id = e->Value();
	string file;
	
	PRINT("LuaManager::ReadXml " + id);
	
	//<script id="something" file="script.lua" md5="hash" active="0" />
	if (id == "script")
	{
		if (online)
			file = DIR_CACHE;
		else
			file = DIR_EDITOR;
		
		file += DIR_SCRIPTS + xf->GetParamString(e, "file");
		
		if ( !Load(file) )
			return XMLPARSE_CANCEL;
	}

	PRINT("LuaManager::ReadXml " + id + " End");
	
	return XMLPARSE_SUCCESS;
}

