
#ifndef _AVATARPROPERTIES_H_
#define _AVATARPROPERTIES_H_

#include "../core/Common.h"

struct lua_State;
class AvatarProperties
{
  public:
	string url, pass;
	int w, h, delay, flags;
	
	// Pulls data from the table at the specific index. -1 for stack top
	bool FromLuaTable(lua_State* ls, int virtualIndex = -1);
	
	// Adds a new table to the top of the lua_State's stack.
	bool ToLuaTable(lua_State* ls);
};

#endif //_AVATARPROPERTIES_H_
