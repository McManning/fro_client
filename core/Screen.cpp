
#include <SDL/SDL.h>
#include "Screen.h"
#include "ResourceManager.h"

#ifdef DEBUG
#	define DRAW_RECTS 1
#endif

Screen* g_screen;
Uint32 uScreenFlags;

void SetScreenFlags(Uint32 flags)
{
	uScreenFlags = flags;
}

Screen::Screen() 
	: Image()
{
	Uint32 colorkey;
	SDL_Surface* image;
	SDL_Surface* image2;
	
	mNoDraw = false;
	
	//Since we don't have a screen since now, it's safe to assume SDL hasn't been initialized yet.
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) 
		FATAL(SDL_GetError());
	
	// Set icon. The icon must be 16bit, BMP format. Using pink as a colorkey.
	image = SDL_LoadBMP("assets/icon.bmp");
	if (image)
	{
		colorkey = SDL_MapRGB(image->format, 255, 0, 255);
		SDL_SetColorKey(image, SDL_SRCCOLORKEY, colorkey);  
		
	//	image2 = SDL_DisplayFormat(image);
		//	else //Yes. It matters.
		//		temp = SDL_DisplayFormat(imgf.frames[0].surf);
		
	//	SDL_FreeSurface(image);
		SDL_WM_SetIcon(image, NULL);
	}
	
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	Resize(SCREEN_WIDTH, SCREEN_HEIGHT);
	
	Update();
	
	g_screen = this;
}

Screen::~Screen()
{
	//Image will handle deletion. 
	//TODO: Kill SDL here?
	mImage->refCount--;
	
	g_screen = NULL;
}

#ifdef DRAW_RECTS
std::vector<rect> g_rects;
#endif

void Screen::Flip()
{
#ifdef DRAW_RECTS
	for (int i = 0; i < g_rects.size(); ++i)
		DrawRound(g_rects.at(i), 0, color(255));
		
	g_rects.clear();
#endif
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
	if (!g_screen)
		g_screen = new Screen();

	return g_screen;
}

void Screen::Destroy()
{
	SAFEDELETE(g_screen);	
}

void Screen::AddRect(rect r)
{
	//sdfUpdate();
#ifdef DRAW_RECTS
	g_rects.push_back(r);
#endif
}



