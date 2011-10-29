
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


#ifndef _REMOTEACTOR_H_
#define _REMOTEACTOR_H_

#include "Actor.h" 

class DataPacket;
class RemoteActor : public Actor
{
  public:
	RemoteActor();
	~RemoteActor();
	
	void Render();

	void ReadAvatarFromPacket(DataPacket& data);
	
	bool LoadAvatar(string file, string pass, uShort w, uShort h, uShort delay, uShort flags);
	
	// Set to true if this player is considered "away from keyboard"
	void SetAfk(bool b) { mAfk = b; };
	
	void SetBlocked(bool b);
	bool IsBlocked() const { return mBlocked; };
	
	int LuaSetProp(lua_State* ls, string& prop, int index);
	int LuaGetProp(lua_State* ls, string& prop);
	
	bool mBlocked; //can we hear/see this player?
	bool mAfk;
	string mHostmask; // IRC server unique identifier
};

#endif //_REMOTEACTOR_H_
