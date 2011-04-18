
#include <lua.hpp>
#include "SystemLib.h"
#include "LuaCommon.h"
#include "../core/widgets/MessagePopup.h"
#include "../core/widgets/OpenUrl.h"
#include "../core/widgets/YesNoPopup.h"
#include "../core/widgets/RightClickMenu.h"
#include "../core/io/FileIO.h"
#include "../game/GameManager.h"
#include "../map/Map.h"

const char* const LUA_CRYPT_PASSWORD = "luaisneat";
const int LUA_CRYPT_LENGTH = 50;

struct luaCallback
{
	lua_State* state;
	string func;
	int reference; // in case there's lua userdata
};

// .Print("Message")
int system_Print(lua_State* ls)
{
	PRINT("system_Print");
	luaCountArgs(ls, 1);

	console->AddMessage( lua_tostring(ls, 1) );
	
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

void _doYesNoCallback(luaCallback* data, int result)
{
	if (!data || !data->state) return;

	lua_getglobal(data->state, data->func.c_str()); //get function name

	//if there isn't a function at the top of the stack, we failed to find it
	if (!lua_isfunction(data->state, -1))
	{
		//cannot be luaError because we need to return!
		WARNING("Lua Function " + data->func + " not found during YesNoCallback");
		return;
	}

	lua_pushboolean(data->state, result);

	if (lua_pcall(data->state, 1, 0, 0) != 0)
		console->AddMessage("\\c900 * LUAYESNO [" + data->func + "] " + string(lua_tostring(data->state, -1)));
}

void callback_luaYesNo_Yes(YesNoPopup* yn)
{
	//call associated lua function
	if (yn->userdata)
	{
		_doYesNoCallback((luaCallback*)yn->userdata, 1);
		delete (luaCallback*)yn->userdata;
		yn->userdata = NULL;
	}
}

void callback_luaYesNo_No(YesNoPopup* yn)
{
	//call associated lua function
	if (yn->userdata)
	{
		_doYesNoCallback((luaCallback*)yn->userdata, 0);
		delete (luaCallback*)yn->userdata;
		yn->userdata = NULL;
	}
}

//	.YesNo("title", "message", "callback", LongFormat<1|0>)
//		Will call the callback with one boolean parameter. 
//		If longformat is true, will use a multiline for the message.
int system_YesNo(lua_State* ls)
{
	PRINT("system_YesNo");
	luaCountArgs(ls, 3);
	int numArgs = lua_gettop(ls);
	
	luaCallback* data = new luaCallback;
	data->state = ls;
	data->func = lua_tostring(ls, 3);
	
	bool useMultiline = false;
	if (numArgs > 3)
		useMultiline = lua_toboolean(ls, 4);
	
	YesNoPopup* yn = new YesNoPopup("", lua_tostring(ls, 1), lua_tostring(ls, 2), useMultiline);
	
	yn->userdata = (void*)data;
	yn->onYesCallback = callback_luaYesNo_Yes;
	yn->onNoCallback = callback_luaYesNo_No;
	
	return 0;
}
/*
void callback_LuaRCM_Close(RightClickMenu* rcm)
{
	luaCallback* lc;
	
	// TODO: Unload lua userdata of each item
	for (int i = 0; i < rcm->mCallbacks.size(); ++i)
	{
		if (rcm->mCallbacks.at(i).userdata)
		{
			lc = (luaCallback*)rcm->mCallbacks.at(i).userdata;
			
			luaL_unref(lc->state, LUA_REGISTRYINDEX, lc->reference);
			delete lc;
		}
	}
}

void callback_LuaRCM_Select(RightClickMenu* rcm, void* userdata)
{
	luaCallback* lc = (luaCallback*)userdata;
	if (lc)
	{
		// call associated lua function
		lua_getglobal(lc->state, lc->func); //get function name

		//if there isn't a function at the top of the stack, we failed to find it
		if (!lua_isfunction(lc->state, -1))
		{
			//cannot be luaError because we need to return!
			WARNING("Lua Function " + lc->func + " not found during RCM Callback");
			return 0;
		}

		if (lc->reference != LUA_BADREF)
			lua_rawgeti(lc->state, LUA_REGISTRYINDEX, lc->reference);
		else
			lua_pushnil(lc->state);
			
		if (lua_pcall(lc->state, 1, 0, 0) != 0)
		{
			console->AddMessage("\\c900 * LUARCM [" + lc->func + "] " 
								+ string(lua_tostring(lc->state, -1)));
		}
	}
}

// ptr = .NewRightClickMenu()
int system_NewRightClickMenu(lua_State* ls)
{
	luaCountArgs(ls, 2);
	RightClickMenu* rcm = new RightClickMenu();
	rcm->onCloseCallback = callback_LuaRCM_Close;
	
	lua_pushlightuserdata(ls, rcm);
	return 1;
}

// .AddToRightClickMenu(ptr, "Text", "lua_callback", userdata)
int system_AddToRightClickMenu(lua_State* ls)
{
	luaCountArgs(ls, 3);
	int numArgs = lua_gettop(ls);
	
	RightClickMenu* rcm = (RightClickMenu*)lua_touserdata(ls, 1);
	if (!rcm)
		return luaError(ls, "System.AddToRightClickMenu", "Invalid RCM");
		
	luaCallback* lc = new luaCallback;
	lc->luaState = ls;
	lc->luaFunc = lua_tostring(ls, 3);
	
	if (numArgs > 3) // has userdata
	{
		lua_pushvalue(ls, 4); //copy the value at index to the top of the stack
		lc->reference = luaL_ref(ls, LUA_REGISTRYINDEX); // creates reference and pops
	}
	else
	{
		lc->reference = LUA_BADREF;
	}
	
	rcm->AddOption(lua_tostring(ls, 2), callback_LuaRCM_Select, lc);
	
	return 0;
}
*/
void _doMessagePopupCallback(luaCallback* data)
{
	if (!data || !data->state) return;

	lua_getglobal(data->state, data->func.c_str()); //get function name

	//if there isn't a function at the top of the stack, we failed to find it
	if (!lua_isfunction(data->state, -1))
	{
		//cannot be luaError because we need to return!
		WARNING("Lua Function " + data->func + " not found during MessagePopupCallback");
		return;
	}

	if (lua_pcall(data->state, 0, 0, 0) != 0)
		console->AddMessage("\\c900 * LUAMSGPOPUP [" + data->func + "] " + string(lua_tostring(data->state, -1)));
}

void callback_luaMessagePopup(MessagePopup* mp)
{
	//call associated lua function
	if (mp->userdata)
	{
		_doMessagePopupCallback((luaCallback*)mp->userdata);
		delete (luaCallback*)mp->userdata;
		mp->userdata = NULL;
	}
}

// .Alert("Title", "Message", "callback"<optional>)
//		Will call the callback lua function (if it exists) with no parameters once closed. 
int system_Alert(lua_State* ls)
{
	PRINT("system_Alert");
	luaCountArgs(ls, 2);
	int numArgs = lua_gettop(ls);
	
	MessagePopup* mp = new MessagePopup("", lua_tostring(ls, 1), lua_tostring(ls, 2), false);
	
	if (numArgs > 2)
	{
		luaCallback* data = new luaCallback;
		data->state = ls;
		data->func = lua_tostring(ls, 3);
			
		mp->userdata = (void*)data;
		mp->onCloseCallback = callback_luaMessagePopup;
	}

	return 0;
}

// .MessageDialog("Title", "Message", "callback"<optional>)
//		Will call the callback lua function (if it exists) with no parameters once closed. 
int system_MessageDialog(lua_State* ls)
{
	PRINT("system_MessageDialog");
	luaCountArgs(ls, 2);
	int numArgs = lua_gettop(ls);
	
	MessagePopup* mp = new MessagePopup("", lua_tostring(ls, 1), lua_tostring(ls, 2), true);
	
	if (numArgs > 2)
	{
		luaCallback* data = new luaCallback;
		data->state = ls;
		data->func = lua_tostring(ls, 3);
			
		mp->userdata = (void*)data;
		mp->onCloseCallback = callback_luaMessagePopup;
	}

	return 0;
}

// bool = .Wildmatch("pattern", "message") - Returns true if the message matches the pattern
int system_Wildmatch(lua_State* ls)
{
	PRINT("system_Wildmatch");
	luaCountArgs(ls, 2);

	lua_pushboolean( ls, wildmatch(lua_tostring(ls, 1), lua_tostring(ls, 2)) );
	
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

//	bool = .Encrypt("file") - Encrypts the file WORKING_DIR/file
int system_Encrypt(lua_State* ls)
{
	PRINT("system_Encrypt");
	luaCountArgs(ls, 1);
	
	string file = game->mMap->mWorkingDir + lua_tostring(ls, 1);

	bool result = encryptFile(file, file + "_", LUA_CRYPT_PASSWORD, LUA_CRYPT_LENGTH);
	copyFile(file + "_", file);
	removeFile(file + "_");
	
	lua_pushboolean(ls, result);
	return 1;
}

//	bool = .Decrypt("file") - Decrypts the file WORKING_DIR/file
int system_Decrypt(lua_State* ls)
{
	PRINT("system_Decrypt");
	luaCountArgs(ls, 1);
	
	string file = game->mMap->mWorkingDir + lua_tostring(ls, 1);

	bool result = decryptFile(file, file + "_", LUA_CRYPT_PASSWORD, LUA_CRYPT_LENGTH);
	copyFile(file + "_", file);
	removeFile(file + "_");
	
	lua_pushboolean(ls, result);
	return 1;
}

// .SetGuiColor(r, g, b)
int system_SetGuiColor(lua_State* ls)
{
	color c;
	luaCountArgs(ls, 3);
	c.r = (int)lua_tonumber(ls, 1);
	c.g = (int)lua_tonumber(ls, 2);
	c.b = (int)lua_tonumber(ls, 3);
	gui->ColorizeGui(c);
	
	return 1;
}

static const luaL_Reg functions[] = {
	{"Print", system_Print},
	{"Fatal", system_Fatal},
	{"YesNo", system_YesNo},
//	{"NewRightClickMenu", system_NewRightClickMenu},
//	{"AddToRightClickMenu", system_AddToRightClickMenu},
	{"MessageDialog", system_MessageDialog},
	{"Alert", system_Alert},
	{"Wildmatch", system_Wildmatch},
	{"OpenUrl", system_OpenUrl},
	{"GetTheta", system_GetTheta},
	{"GetDistance", system_GetDistance},
	{"OffsetByTheta", system_OffsetByTheta},
	{"StringToNumber", system_StringToNumber},
	{"GenerateFilename", system_GenerateFilename},
	{"Encrypt", system_Encrypt},
	{"Decrypt", system_Decrypt},
	{"SetGuiColor", system_SetGuiColor},
	{NULL, NULL}
};

void RegisterSystemLib(lua_State* ls)
{
	luaL_register( ls, "System", functions );
}
