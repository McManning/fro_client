
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
#include "KeyValueMap.h"
#include "Crypt.h"


// Create a lua_State and load our helper interface functions into it
KeyValueMap::KeyValueMap()
{
	mUseEncryption = false;
	mLuaState = luaL_newstate();
	luaL_openlibs( mLuaState ); 
	
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
int KeyValueMap::Load(string file, bool useEncryption)
{
	mFilename = file;
	mUseEncryption = useEncryption;
		
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
	if (mUseEncryption)
	{
		section = base64_encode(section.c_str(), section.length());
		key = base64_encode(key.c_str(), key.length());
	}
	
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
	
	if (mUseEncryption)
		result = base64_decode(result.c_str(), result.length());
		
	return result;
}

// Call SetValue() in lua
int KeyValueMap::SetValue(string section, string key, string value)
{
	if (mUseEncryption)
	{
		section = base64_encode(section.c_str(), section.length());
		key = base64_encode(key.c_str(), key.length());
		if (!value.empty())
			value = base64_encode(value.c_str(), value.length());
	}
	
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
