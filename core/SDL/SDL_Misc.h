
#ifndef _SDL_MISC_H_
#define _SDL_MISC_H_

#include <SDL/SDL.h>
#include "../Common.h"


//Only use these macros internally!

#define COLORTOUINT32(_image, _color) \
	SDL_MapRGBA(_image->surf->format, _color.r, _color.g, _color.b, _color.a)

#define LOCKSURF(_surf) \
	if ( SDL_MUSTLOCK(_surf) ) \
	{ \
		if ( SDL_LockSurface(_surf) < 0) { \
			FATAL("Lock Failed: " + string(SDL_GetError())); \
		} \
	}

#define UNLOCKSURF(_surf) \
	if ( SDL_MUSTLOCK(_surf) ) { \
        SDL_UnlockSurface(_surf); \
    }

//Extra SDL Routines

//Byte order table
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    #define RMASK 0xff000000
    #define GMASK 0x00ff0000
    #define BMASK 0x0000ff00
    #define AMASK 0x000000ff
#else
    #define RMASK 0x000000ff
    #define GMASK 0x0000ff00
    #define BMASK 0x00ff0000
    #define AMASK 0xff000000
#endif

Uint32 SDL_GetPixel(SDL_Surface* surf, Uint32 x, Uint32 y);

bool SDL_SetPixel(SDL_Surface* surf, Uint32 x, Uint32 y, 
					Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool copy);

SDL_Surface* SDL_NewSurface(Uint32 width, Uint32 height, color colorKey, bool alphaChannel);

/*	Creates an exact duplication of the given rect as a new SDL_Surface */
SDL_Surface* SDL_CopySurface(SDL_Surface* src, SDL_Rect r);

#endif //_SDL_MISC_H_
