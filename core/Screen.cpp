
#include <SDL/SDL.h>
#include "Screen.h"
#include "ResourceManager.h"

Screen* gScreen;
Uint32 uScreenFlags;

void SetScreenFlags(Uint32 flags)
{
	uScreenFlags = flags;
}

Screen::Screen() 
	: Image()
{
	//Since we don't have a screen since now, it's safe to assume SDL hasn't been initialized yet.
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) 
		FATAL(SDL_GetError());

	SDL_WM_SetIcon(SDL_LoadBMP("assets/icon.bmp"), NULL);
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	Resize(SCREEN_WIDTH, SCREEN_HEIGHT);
	
	Update();
}

Screen::~Screen()
{
	//Image will handle deletion. 
	//TODO: Kill SDL here?
	mImage->refCount--;
}

void Screen::Flip()
{
	SDL_Flip(Surface());
	
#ifdef OPTIMIZED
	mNeedUpdate = false;
#endif
}

void Screen::Resize(uShort w, uShort h)
{
	ASSERT(w > 0 && h > 0);
	
	SDL_Surface* surf = SDL_SetVideoMode(w, h, 32, uScreenFlags);
	if (!surf)
	{
		FATAL(SDL_GetError());
	}
	
	if (mImage)
		mImage->refCount--;
		
	SAFEDELETE(mImage);
	mImage = resman->SDLImgFromSurface(surf);
	
	Update();
}

Screen* Screen::Instance()
{
	if (!gScreen)
		gScreen = new Screen();

	return gScreen;
}

void Screen::Destroy()
{
	SAFEDELETE(gScreen);
}
