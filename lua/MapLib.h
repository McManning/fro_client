
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


#ifndef _LUA_MAPLIB_H_
#define _LUA_MAPLIB_H_

#include "../core/Core.h"

/*	Called in WorldLoader if the load fails, or ~Map when the map is destroyed */
void mapLib_CloseLuaState(lua_State*);

/*	Called in WorldLoader to initialize a state for us to run as the main map script */
lua_State* mapLib_OpenLuaState();

/*	Calls Build() in the lua script. Return 0 on error */
int mapLib_luaCallBuildWorld(lua_State*, bool, string&, std::vector<string>&);

void mapLib_CallCheckin(lua_State* ls, string& filename);

/*	Calls Display() in the lua script when the map has been loaded fully and displayed onscreen */
void mapLib_CallDisplay(lua_State*);

void RegisterMapLib(lua_State*);

#endif //_LUA_MAPLIB_H_
