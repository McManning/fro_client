
#include "Avatar.h"
#include "../core/io/FileIO.h"
#include "../core/net/DataPacket.h"
#include "../game/GameManager.h"

Avatar::Avatar()
{
	mImage = mDisplayedImage = NULL;
	mModifier = MOD_NONE;
}

Avatar::~Avatar()
{
	if (mDisplayedImage != mImage)
		resman->Unload(mDisplayedImage);
		
	resman->Unload(mImage);
}

Avatar& Avatar::operator=(const Avatar& a)
{
	if (a.mDisplayedImage && a.mDisplayedImage != a.mImage)
		mDisplayedImage = a.mDisplayedImage->Clone();

	//just do a shallow copy of mImage, since no Avatar class can modify it
	mImage = a.mImage->Clone();
	
	mId = a.mId;
	mUrl = a.mUrl;
	mPass = a.mPass;
	mWidth = a.mWidth;
	mHeight = a.mHeight;
	mDelay = a.mDelay;
	mLoopStand = a.mLoopStand;
	mLoopSit = a.mLoopSit;
	mModifier = a.mModifier;
}

bool Avatar::Modify(byte modifier)
{
	RevertModification();
	
	if (modifier != MOD_NONE)
	{	
		mDisplayedImage = mImage->Clone(true); //create a deep copy of mImage
		if (!mDisplayedImage)
			return false;

		switch (modifier)
		{
			case MOD_MINI: //Cut the avatars size in half
				DEBUGOUT("Applying MOD_MINI To Avatar");
				mDisplayedImage->Rotate(0.0, 0.5, 1);
				break;
			case MOD_GIANT:
				DEBUGOUT("Applying MOD_GIANT To Avatar");
				mDisplayedImage->Rotate(0.0, 2.0, 1);
				break;
			case MOD_SMASH:
				DEBUGOUT("Applying MOD_SMASH To Avatar");
				mDisplayedImage->Scale(1.0, 0.3, 1);
				break;
			case MOD_GHOST:
				DEBUGOUT("Applying MOD_GHOST To Avatar");
				mDisplayedImage->ReduceAlpha(200);
				break;
			default: 
				console->AddMessage("\\c900 * Unknown Avatar Modifier: " + its(modifier)); 
				return false;
				break;
		}
	}
	
	mModifier = modifier;
	return true;
}

void Avatar::RevertModification()
{
	if (mDisplayedImage && mDisplayedImage != mImage)
	{	
		resman->Unload(mDisplayedImage);
		mDisplayedImage = NULL;
	}
	mModifier = MOD_NONE;
}

bool Avatar::Convert()
{
	//TODO: Remove this GIF-block when we fix gif crashers
	if (mImage->mImage->format == IMG_FORMAT_GIF)
	{
		console->AddMessage("\\c900* Blocked GIF Avatar");
		return false;
	}
	
	bool result = false;
	result = mImage->ConvertToAvatarFormat(mWidth, mHeight, mDelay, mLoopStand, mLoopSit);
	return result;
}

void Avatar::Serialize(DataPacket& dst)
{
	dst.WriteString(mUrl);
	dst.WriteInt(mWidth);
	dst.WriteInt(mHeight);
	dst.WriteInt(mDelay);
	dst.WriteInt(mLoopStand);
	dst.WriteInt(mLoopSit);
	dst.WriteInt(mModifier);
	dst.WriteString(mPass);
}

void Avatar::Deserialize(DataPacket& src, uShort offset)
{
	if (src.Size() <= offset + 5)
		return;
		
	mUrl = src.ReadString(offset);
	mWidth = src.ReadInt(offset+1);
	mHeight = src.ReadInt(offset+2);
	mDelay = src.ReadInt(offset+3);
	mLoopStand = src.ReadInt(offset+4);
	mLoopSit = src.ReadInt(offset+5);
			
	//Following two members don't exist in the older client versions
			
	//TODO: Figure out a way to apply this modifier after load, internally!
	if (src.Size() > offset+6)
		mModifier = src.ReadInt(offset+6);
	else
		mModifier = MOD_NONE;
		
	if (src.Size() > offset+7)
		mPass = src.ReadString(offset+7);
	else
		mPass.clear();

	resman->Unload(mImage);
	resman->Unload(mDisplayedImage);
	mDisplayedImage = NULL;
	
	mImage = resman->LoadImg(mUrl, "", mPass);
}

void Avatar::Load()
{
	resman->Unload(mImage);
	resman->Unload(mDisplayedImage);
	mDisplayedImage = mImage = NULL;
	
	if (mUrl.find("avy://", 0) == 0)
	{
		DEBUGOUT("Loading as composite");
		LoadComposite();	
	}
	else
	{
		mImage = resman->LoadImg(mUrl, "", mPass);
	}
}

/*	Load an avatar in the form of avy://Base.HeadId.HeadHex.BodyId.BodyHex.HairId.HairHex */
void Avatar::LoadComposite()
{
	string url = mUrl.substr(5); //cut off avy://
	
	vString v;
	explode(&v, &url, ".");
	
	if (v.size() < 7) //TODO: Some sort of error thing here to tell the actor it's bad?
		return;
	
	string base = v.at(0);
	string sHead = v.at(1);
	string sBody = v.at(3);
	string sHair = v.at(5);
	color cHead = hexToColor(v.at(2));
	color cBody = hexToColor(v.at(4));
	color cHair = hexToColor(v.at(6));
	
	//If any of these colors are greyscale, re-adjust them slightly to avoid conflict with the colorize routine
	if (isGreyscale(cHead)) cHead.r += (cHead.r > 0) ? -1 : 1;
	if (isGreyscale(cBody)) cBody.r += (cBody.r > 0) ? -1 : 1;
	if (isGreyscale(cHair)) cHair.r += (cHair.r > 0) ? -1 : 1;
	
	Image* iHead = resman->LoadImg(DIR_AVA + base + ".head." + sHead + ".png");
	Image* iBody = resman->LoadImg(DIR_AVA + base + ".body." + sBody + ".png");
	Image* iHair = resman->LoadImg(DIR_AVA + base + ".hair." + sHair + ".png");
	
	//if we are lacking a resource, bad avy.
	if (!iHead || !iBody || !iHair)
		return;
		
	//calculate our dimensions based on the body. (All parts should have the same dimensions with this system)
	//Each should contain 2 frames, 5 rows (4 dirs and 1 sit)
	mWidth = iBody->Width() / 2;
	mHeight = iBody->Height() / 5;

	//build the avatar, render each part, and colorize as we go along.
	mImage = resman->NewImage(iBody->Width(), iBody->Height(), color(255,0,255), true);

	iHead->Render(mImage, 0, 0);
	mImage->ColorizeGreyscale(cHead);
	resman->Unload(iHead);

	iBody->Render(mImage, 0, 0);
	mImage->ColorizeGreyscale(cBody);
	resman->Unload(iBody);

	iHair->Render(mImage, 0, 0);
	mImage->ColorizeGreyscale(cHair);
	resman->Unload(iHair);
}

void Avatar::ToFiles()
{
	string dir = "saved/avatar_" + timestamp(true) + "/";

	if (!buildDirectoryTree(dir))
		return;
	string file;
	
	file = dir + "info.txt";
	FILE* f = fopen(file.c_str(), "w");
	
	SDL_Image* img = GetImage()->mImage;
	
	fprintf(f, "Original File: %s - Format: %i\n", img->filename.c_str(), img->format);
	
	for (int i = 0; i < img->framesets.size(); i++)
	{
		fprintf(f, "Frameset %s - Loop: %i\n", img->framesets.at(i).key.c_str(), img->framesets.at(i).loop);
		for (int ii = 0; ii < img->framesets.at(i).frames.size(); ii++)
		{
			file = dir + img->framesets.at(i).key + "." + its(ii) + ".png";
			fprintf(f, "\t%i: [%s] %s\n", ii, img->framesets.at(i).frames.at(ii).key, file.c_str());
			IMG_SavePNG(file.c_str(), img->framesets.at(i).frames.at(ii).surf);
		}
	}
	
	fclose(f);
}


