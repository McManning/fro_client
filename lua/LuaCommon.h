
#ifndef _LUACOMMON_H_
#define _LUACOMMON_H_

#include "../core/Core.h"

typedef enum 
{
	LUATYPE_STRING = 0,
	LUATYPE_INTEGER,
	LUATYPE_USERDATA
} luaDataType;

class lua_State;

// If we don't have the desired amount of arguments, return an error 
bool luaCountArgs(lua_State* ls, int desired);

int luaError(lua_State* ls, string func, string msg);

#endif //_LUACOMMON_H_
