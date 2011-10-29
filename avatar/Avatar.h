
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


#ifndef _AVATAR_H_
#define _AVATAR_H_

#include "../core/Core.h"
 
const int AVATAR_FLAG_LOOPSTAND = 1;
const int AVATAR_FLAG_LOOPSIT = 2;

const int MAX_AVATAR_FILESIZE = (200 * 1024); //200 KB

#ifdef DEBUG
const int MAX_AVATAR_WIDTH = 64*10;
const int MAX_AVATAR_HEIGHT = 96*10;
#else
const int MAX_AVATAR_WIDTH = 64;
const int MAX_AVATAR_HEIGHT = 96;
#endif

class DataPacket;
class Avatar
{
  public:
	Avatar();
	~Avatar();	

	void Serialize(DataPacket& dst);

	/*	Will load an avatar based on our current properties */
	void Load();
	
	/*	Load an avatar in the form of avy://Base.HeadId.HeadHex.BodyId.BodyHex.HairId.HairHex */
	void LoadComposite();
	
	/*	Called after the image has been downloaded and loaded into mImage.
		Will perform modifications to it, convert to proper avatar format, etc.
		Returns false on any failures
	*/
	bool Convert();
	
	/*	Will output all frames of this avatar to PNG files for debugging purposes */
	void ToFiles();
	
	bool Modify(byte modifier);
	void RevertModification();
	
	enum { //avatar modifiers
		MOD_NONE = 0,
		MOD_TARANDFEATHER,
		MOD_MINI,
		MOD_GIANT,
		MOD_SMASH,
		MOD_GHOST,
	};
	
	enum avatarState
	{
		LOADING = 0,
		LOADED,
		FAILED,
		BADIMAGE
	};

	avatarState mState;
	
	Image* GetImage() { return (mDisplayedImage) ? mDisplayedImage : mImage; };
	
	Image* mDisplayedImage; //the version we show, if different from mImage
	Image* mImage; //the original, loaded-from-file version
	string mId;
	string mError;
	
	//Properties that will be (de)serialized
	string mUrl;
	string mPass;
	uShort mWidth;
	uShort mHeight;
	uShort mDelay;
	uShort mFlags;
	
	byte mModifier;
};

#endif //_AVATAR_H_
