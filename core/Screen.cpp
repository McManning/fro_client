
#include <SDL/SDL.h>
#include "Screen.h"
#include "ResourceManager.h"

Screen* g_screen;
RectManager g_RectMan;
Uint32 uScreenFlags;

std::vector<rect> g_rects;

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
	mOptimizationMethod = FULL_OPTIMIZATION;
	
#ifdef DEBUG
	mDrawOptimizedRects = true;
#else
    mDrawOptimizedRects = false;
#endif

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

void Screen::PreRender()
{
	// add the lazy rect to the optimizer
	if (mOptimizationMethod == LAZY_OPTIMIZATION)
	{
		g_RectMan.add_rect(mLazyRect);
		mLazyRect.w = mLazyRect.h = 0;
	}
	
	if (mOptimizationMethod != NO_OPTIMIZATION)
	{
	    if (mDrawOptimizedRects)
		   g_rects.clear();
	
		g_RectMan.generate_clips(Surface());
	}
}

void Screen::PostRender()
{

//	for (int i = 0; i < g_rects.size(); ++i)
//		DrawRound(g_rects.at(i), 0, color(255));

	//g_RectMan.rects_print(g_RectMan.m_RectSet);
	
	if (mOptimizationMethod != NO_OPTIMIZATION)
	{
		if (g_RectMan.m_Clips)
		{
	        if (mDrawOptimizedRects)
	        {
	    		SDL_Rect* r;
	    		for (int i = 0; i < g_RectMan.m_Clips->length; ++i)
	    		{
	    			r = &g_RectMan.m_Clips->rects + i;
	    			DrawRound(rect(r->x, r->y, r->w, r->h), 0, color(0,255,0,100));	
	    		}
	        }
	
			g_RectMan.update_rects(Surface());
			
			// Now that we've done that, black out the old rects so we know they were
			// still there, but don't get confused with new rects added each frame
			/*for (int i = 0; i < g_RectMan.m_Clips->length; ++i)
			{
				r = &g_RectMan.m_Clips->rects + i;
				DrawRound(rect(r->x, r->y, r->w, r->h), 0, color());	
			}*/
			
			g_RectMan.purge();
		}
	}
	else if (mNeedUpdate)
	{
		SDL_Flip(Surface());
	}

	mNeedUpdate = false;
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
	
	if (SDL_GetAppState() & SDL_APPACTIVE)
	{
		if (mOptimizationMethod == FULL_OPTIMIZATION)
		{
			if (mDrawOptimizedRects)
				g_rects.push_back(r);
			
			SDL_Rect sr = { r.x, r.y, r.w, r.h };
			g_RectMan.add_rect(sr);
		}
		else if (mOptimizationMethod == LAZY_OPTIMIZATION) 
		{
			// first added
			if (mLazyRect.w == 0 && mLazyRect.h == 0)
			{
				mLazyRect.x = r.x;
				mLazyRect.y = r.y;
				mLazyRect.w = r.w;
				mLazyRect.h = r.h;
			}
			else // union the lazy one with the new one
			{
				int i = MIN(r.x, mLazyRect.x);
				mLazyRect.w = MAX(r.x + r.w, mLazyRect.x + mLazyRect.w) - i;
				mLazyRect.x = i;
				
				i = MIN(r.y, mLazyRect.y);
				mLazyRect.h = MAX(r.y + r.h, mLazyRect.y + mLazyRect.h) - i;
				mLazyRect.y = i;
			}
		}
		else
		{
			mNeedUpdate = true;
		}
	}
}

bool Screen::IsRectDrawable(rect r)
{
	return ((mOptimizationMethod == NO_OPTIMIZATION && mNeedUpdate)
			|| (g_RectMan.m_RectSet && 
				g_RectMan.rects_intersects(g_RectMan.m_RectSet, r.x, r.y, r.w, r.h) == _NO_ERR)
			);
}

