
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


#include "ResourceManager.h"
#include "net/DownloadManager.h"
#include "SDL/SDL_Misc.h"
#include "SDL/SDL_rotozoom.h"
#include "io/FileIO.h"
#include "Logger.h"

ResourceManager* resman;

ResourceManager::ResourceManager()
{
	resman = this;
}

ResourceManager::~ResourceManager()
{
	logger.Write("Unloading SDL_Images");
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
}

SDL_Image* ResourceManager::Load(string file, string password)
{
	//check if it's loaded yet, if so, ++reference and return
	for (int i = 0; i < mImages.size(); ++i)
	{
		if (mImages.at(i)->filename == file)
		{
		//	DEBUGOUT("Found match for " + file + " refcount: " + its(mImages.at(i)->refCount));
			mImages.at(i)->refCount++;
			return mImages.at(i);
		}
	}

	SDL_Image* img = new SDL_Image;
	img->password = password;
	img->filename = file;

	if ( !img->Load(file, password) )
	{
		WARNING("Could not load SDL_Image " + file);
		delete img;
		return NULL;
	}

	//img->PrintInfo();
	img->managed = true;
	mImages.push_back(img);

	return img;
}

bool ResourceManager::Unload(SDL_Image* src)
{
    if (src)
    {
        src->refCount--;
        if (src->refCount < 1)
        {
            if (src->managed) // don't search if we're not managing it
            {
                // erase from managed list
                for (int i = 0; i < mImages.size(); i++)
                {
                    if (mImages.at(i) && mImages.at(i) == src)
                    {
                        mImages.erase(mImages.begin() + i);
                        break;
                    }
                }
            }

            delete src;
            return true;
        }
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

//	DEBUGOUT("[ResourceManager::Unload Image*] Deleting Image* " + pts(src) + " File: " + src->Filename());
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
	DEBUGOUT("[FramesToImage] File: " + filename + " count: " + its(count));
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
	return true;
}

Image* ResourceManager::ImageFromSurface(SDL_Surface* src)  //to resman
{
	SDL_Frame* frame = new SDL_Frame;
	frame->delay = 0;
	frame->key = NULL;
	frame->surf = src;

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

void ResourceManager::ReloadFromDisk(SDL_Image* img)
{
	if ( !img->Load(img->filename, img->password) )
	{
		WARNING("[ResourceManager::ReloadFromDisk] Could not reload " + img->filename);
	}
}

