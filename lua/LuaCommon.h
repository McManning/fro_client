
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

int luaCall(lua_State* ls, int a, int b);

int luaError(lua_State* ls, string func, string msg);

void luaStackdump(lua_State* l);

#endif //_LUACOMMON_H_
