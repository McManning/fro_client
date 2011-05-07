
#ifndef _LUACOMMON_H_
#define _LUACOMMON_H_

#include "../core/Core.h"

/*
// Table management shorthand
#define LUAT_ADD_STRING(_key, _string) {\
		lua_pushstring(ls, _key); \
		lua_pushstring(ls, _string.c_str()); \
		lua_settable(ls, top); \
	}
	
// Table management shorthand
#define LUAT_ADD_INT(_key, _int) {\
		lua_pushstring(ls, _key); \
		lua_pushnumber(ls, _int); \
		lua_settable(ls, top); \
	}
*/

class lua_State;

// If we don't have the desired amount of arguments, return an error 
bool luaCountArgs(lua_State* ls, int desired);

int luaError(lua_State* ls, string func, string msg);

void luaStackdump(lua_State* l);

#endif //_LUACOMMON_H_
