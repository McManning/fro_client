
#include <lua.hpp>
#include "KeyValueMap.h"
#include "Crypt.h"


// Create a lua_State and load our helper interface functions into it
KeyValueMap::KeyValueMap()
{
	mLuaState = luaL_newstate();

	if ( luaL_dofile( mLuaState, "assets/lua/KeyValueMap.lua" ) != 0 )
	{
		string err = lua_tostring(mLuaState, -1);
		FATAL("Failed to create KeyValueMap: " + err);
	}
}

KeyValueMap::~KeyValueMap()
{
	if (mLuaState)
		lua_close(mLuaState);
}

// Call LoadDataMap() in lua
int KeyValueMap::Load(string file)
{
	mFilename = file;
		
	lua_getglobal(mLuaState, "LoadDataMap");

	//if there isn't a function at the top of the stack, we failed to find it
	if (!lua_isfunction(mLuaState, -1))
	{
		FATAL("No LoadDataMap()");
	}

	lua_pushstring(mLuaState, file.c_str());

	int result = 0;
	if (lua_pcall(mLuaState, 1, 1, 0) != 0)
	{
		FATAL(lua_tostring(mLuaState, -1));
	}
	
	if (lua_isnumber(mLuaState, -1))
		result = (int)lua_tonumber(mLuaState, -1);

	lua_pop(mLuaState, 1);
	return result;
}

// Call SaveDataMap() in lua
int KeyValueMap::Save()
{
	lua_getglobal(mLuaState, "SaveDataMap");

	//if there isn't a function at the top of the stack, we failed to find it
	if (!lua_isfunction(mLuaState, -1))
	{
		FATAL("No SaveDataMap()");
	}

	lua_pushstring(mLuaState, mFilename.c_str());

	int result = 0;
	if (lua_pcall(mLuaState, 1, 1, 0) != 0)
	{
		FATAL(lua_tostring(mLuaState, -1));
	}
	
	if (lua_isnumber(mLuaState, -1))
		result = (int)lua_tonumber(mLuaState, -1);

	lua_pop(mLuaState, 1);
	return result;
}

// Call GetValue() in lua
string KeyValueMap::GetValue(string section, string key)
{
	section = base64_encode(section.c_str(), section.length());
	key = base64_encode(key.c_str(), key.length());

	lua_getglobal(mLuaState, "GetValue");

	//if there isn't a function at the top of the stack, we failed to find it
	if (!lua_isfunction(mLuaState, -1))
	{
		FATAL("No GetValue()");
	}

	lua_pushstring(mLuaState, section.c_str());
	lua_pushstring(mLuaState, key.c_str());

	string result;
	if (lua_pcall(mLuaState, 2, 1, 0) != 0)
	{
		FATAL(lua_tostring(mLuaState, -1));
	}
	
	result = lua_tostring(mLuaState, -1);
	lua_pop(mLuaState, 1);
	
	result = base64_decode(result.c_str(), result.length());
	return result;
}

// Call SetValue() in lua
int KeyValueMap::SetValue(string section, string key, string value)
{
	section = base64_encode(section.c_str(), section.length());
	key = base64_encode(key.c_str(), key.length());
	if (!value.empty())
		value = base64_encode(value.c_str(), value.length());
	
	lua_getglobal(mLuaState, "SetValue");

	//if there isn't a function at the top of the stack, we failed to find it
	if (!lua_isfunction(mLuaState, -1))
	{
		FATAL("No SetValue()");
	}

	lua_pushstring(mLuaState, section.c_str());
	lua_pushstring(mLuaState, key.c_str());
	lua_pushstring(mLuaState, value.c_str());

	int result = 0;
	if (lua_pcall(mLuaState, 3, 1, 0) != 0)
	{
		FATAL(lua_tostring(mLuaState, -1));
	}
	
	if (lua_isnumber(mLuaState, -1))
		result = (int)lua_tonumber(mLuaState, -1);
		
	lua_pop(mLuaState, 1);
	return result;
}
