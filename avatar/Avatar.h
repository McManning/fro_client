
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