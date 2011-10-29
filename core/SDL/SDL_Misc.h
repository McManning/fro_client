
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

void SDL_Colorize(SDL_Surface* src, Uint8 r, Uint8 g, Uint8 b);

#endif //_SDL_MISC_H_
