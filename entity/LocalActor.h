
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


#ifndef _LOCALACTOR_H_
#define _LOCALACTOR_H_

#include "Actor.h" 

#define DEFAULT_ACTION_BUFFER_SEND_DELAY 3000

class LocalActor : public Actor
{
  public:
	LocalActor();
	~LocalActor();
	
	void Render();
	
	/* Overloaded to do _checkInput when we are capable of accepting new input */
	bool ProcessMovement();
	void PostMovement();
	bool StepForward();
	
	/* Overloaded to duplicate the data into mOutputActionBuffer */
	void AddToActionBuffer(string data);
	void ClearActionBuffer() { mActionBuffer.clear(); mOutputActionBuffer.clear(); };

	/*	Overloaded to save our avatar to GameManager's xml config */
	bool LoadAvatar(string file, string pass, uShort w, uShort h, uShort delay, uShort flags);

	void AvatarError(int err);

	/*	index - Index of the stack where our new value for the property should be */
	int LuaSetProp(lua_State* ls, string& prop, int index);
	int LuaGetProp(lua_State* ls, string& prop);
					
	void SetFlag(string flag, string value);
	string GetFlag(string flag);
	void LoadFlags();
	void SaveFlags();
	void PrintFlags();
	
	void Warp(string id, point2d position, string targetObjectName);
	void Warp(point2d pos); //override to warp on the same map
	
	/* Information that was sent out in the last netSendActionBuffer */
	point2d mLastSavedPosition;
	byte mLastSavedSpeed;
	
	/* 	flag that, if trying to send the buffer mid-jump (or similar state), 
		is set to true and sends buffer out after landing. */
	bool mNeedsToSendBuffer; 
	
	string mOutputActionBuffer; //outgoing action data
	
	//keeps track of when to send mOutputActionBuffer over the network
	timer* mActionBufferTimer;

	bool mIsLocked; //if true, user input is ignored

	int mActionBufferSendDelayMs;
	
	bool mCanChangeAvatar;

  private:
	void _checkInput();
	
	bool mShiftDown;
};

#endif //_LOCALACTOR_H_
