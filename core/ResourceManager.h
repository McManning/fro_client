
#ifndef _RESOURCEMANAGER_H_
#define _RESOURCEMANAGER_H_

#include "Image.h"

class SDL_Image;
class ResourceManager
{
  public:
	ResourceManager();
	~ResourceManager();
	
	SDL_Image* Load(string file, string password = "");
	bool Unload(SDL_Image* src);
	
	Image* LoadImg(string file, string password = "");
	bool Unload(Image* src);

	Image* NewImage(uShort w, uShort h, color colorKey, bool hasAlphaChannel);
	
	//TODO: Move elsewhere?
	bool FramesToImage(SDL_Image* img, SDL_Frame* frames, uShort count, string filename);
	Image* ImageFromSurface(SDL_Surface* src);
	SDL_Image* SDLImgFromSurface(SDL_Surface* surf);
	
	void ReloadFromDisk(SDL_Image* img);
	
	std::vector<SDL_Image*> mImages;
};

extern ResourceManager* resman;

#endif //_RESOURCEMANAGER_H_
