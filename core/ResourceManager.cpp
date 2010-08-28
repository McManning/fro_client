
#include "ResourceManager.h"
#include "net/DownloadManager.h"
#include "SDL/SDL_Misc.h"
#include "SDL/SDL_rotozoom.h"
#include "io/FileIO.h"

ResourceManager* resman;

ResourceManager::ResourceManager()
{
	resman = this;
}

ResourceManager::~ResourceManager()
{
	PRINT("[ResourceManager::~ResourceManager] Unloading SDL_Images");
	int i;
	for (i = 0; i < mImages.size(); i++)
	{
		WARNING("Leaked SDL_Image " + pts(mImages.at(i))
				+ "\n\t File: " + mImages.at(i)->filename 
				+ " References: " + its(mImages.at(i)->refCount)
			);
		delete mImages.at(i);
	}
	mImages.clear();

	resman = NULL;
	PRINT("[ResourceManager::~ResourceManager] Completed");
}

SDL_Image* ResourceManager::Load(string file, string password)
{	
	//check if it's loaded yet, if so, ++reference and return
	for (int i = 0; i < mImages.size(); ++i)
	{
		if (mImages.at(i)->filename == file)
		{
		//	PRINT("Found match for " + file + " refcount: " + its(mImages.at(i)->refCount));
			mImages.at(i)->refCount++;
			return mImages.at(i);
		}
	}
	
	SDL_Image* img = new SDL_Image;
	img->password = password;
	img->filename = file;

	if ( !img->Load(file, password) )
	{
		WARNING("[SDL_Image* ResourceManager::Load] Could not load " + file);
		delete img;
		return NULL;
	}

	//img->PrintInfo();
	mImages.push_back(img);
	
	PRINT("[SDL_Image* ResourceManager::Load] Returning " + pts(img));
	
	return img;
}

bool ResourceManager::Unload(SDL_Image* src)
{
	if (!src)
		return false;
		
	for (int i = 0; i < mImages.size(); i++)
	{
		if (mImages.at(i) && mImages.at(i) == src)
		{
		//	PRINT("[ResourceManager::Unload SDL_Image*] Located Source: " + src->filename 
		//			+ " refcount: " + its(src->refCount));
			src->refCount--;
			if (src->refCount < 1) //no more links, delete
			{
		//		PRINT("[ResourceManager::Unload SDL_Image*] Refcount < 1, deleting");
				delete src;
				mImages.erase(mImages.begin() + i);
				return true;
			}
			return true;
		}
	}
	
	PRINT("SDL_Image Not Managed! " + pts(src) + " file: " + src->filename);
	src->refCount--;
	if (src->refCount < 1) //no more links, delete
	{
//		PRINT("[ResourceManager::Unload SDL_Image*] Unmanaged Refcount < 1, deleting");
		SAFEDELETE(src);
	}
	
	return false;
}

Image* ResourceManager::LoadImg(string file, string password)
{
	SDL_Image* sdlimg = Load(file, password);
	
	if (!sdlimg)
		return NULL;
		
	Image* img = new Image;
	img->mImage = sdlimg;

	return img;
}

bool ResourceManager::Unload(Image* src)
{
	if (!src)
		return false;

//	PRINT("[ResourceManager::Unload Image*] Deleting Image* " + pts(src) + " File: " + src->Filename());
	delete src;

	return true;
}

Image* ResourceManager::NewImage(uShort w, uShort h, color colorKey, bool hasAlphaChannel)
{
	SDL_Surface* surf = SDL_NewSurface(w, h, colorKey, hasAlphaChannel);
	
	//TODO: Manage this?
	return (surf) ? ImageFromSurface(surf) : NULL;
}

bool ResourceManager::FramesToImage(SDL_Image* img, SDL_Frame* frames, uShort count, string filename)
{
	bool found;
	uShort s, i;

	if (!frames || count < 1 || !img)
		return false;

	img->filename = filename;
	PRINT("[FramesToImage] File: " + filename + " count: " + its(count));
	for (i = 0; i < count; i++) //go through all frames and organize
	{
		if ( !frames[i].key) //no key, dump into most recent frameset
		{
			if (img->framesets.empty())
				img->framesets.resize(1);
			img->framesets.at(img->framesets.size() - 1).frames.push_back( frames[i] );
			img->framesets.at(img->framesets.size() - 1).frames.at(
				img->framesets.at(img->framesets.size() - 1).frames.size()-1).key = NULL;
		}
		else //add a new frameset or append to an existing one
		{
			found = false;

			//find out if the frameset already exists, if so, add it
			for (s = 0; s < img->framesets.size(); s++)
			{
				if (img->framesets.at(s).key == frames[i].key)
				{
					free(frames[i].key);
					frames[i].key = NULL;
					img->framesets.at(s).frames.push_back( frames[i] );
					found = true;
					break;
				}
			}
			if (!found) //add as a new frameset
			{
				SDL_Frameset fs;
				
				fs.key = lowercase(frames[i].key); //set key as master for this set
				fs.frames.push_back( frames[i] );
				fs.frames.at(fs.frames.size()-1).key = NULL; //to prevent bad pointers later
				img->framesets.push_back(fs);
			}
		
			//no longer need this frames key
			free(frames[i].key); 
			frames[i].key = NULL;
		}
	}

	free(frames); //we've eaten these all up, we're done.
	PRINT("[FramesToImage] Done");
	
	return true;
}

Image* ResourceManager::ImageFromSurface(SDL_Surface* src)  //to resman
{
	SDL_Frame* frame = new SDL_Frame;
	frame->delay = 0;
	frame->key = NULL;
	frame->surf = src;

	PRINT("[ImageFromSurface]");
	Image* image = new Image();
	image->mImage = new SDL_Image;
	image->mImage->state = SDL_Image::LOADED;
	if (!FramesToImage(image->mImage, frame, 1, ""))
	{
		SAFEDELETE(image->mImage);
		delete image;
		return NULL;
	}

	return image;
}

SDL_Image* ResourceManager::SDLImgFromSurface(SDL_Surface* surf)
{
	SDL_Image* img = new SDL_Image;

	img->framesets.resize(1);
	img->framesets.at(0).frames.resize(1);
	
	img->framesets.at(0).frames.at(0).surf = surf;
	img->framesets.at(0).frames.at(0).key = NULL;
	img->framesets.at(0).frames.at(0).delay = 0;
	
	return img;
}
