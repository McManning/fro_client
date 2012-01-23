
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

#include "SDL_Misc.h"

Uint32 SDL_GetPixel(SDL_Surface* surf, Uint32 x, Uint32 y)
{
	if (!surf) {
//		WARNING("NULL SURF");
		return 0;
	}

	// Somewhere, we didn't lock. That is bad ]:<
	if (!surf->pixels)
	{
		FATAL("SDL_GetPixel called on " + pts(surf) + " without lock.");
	}

	if (x >= surf->w || y >= surf->h || x < 0 || y < 0) {
	//	WARNING("Invalid XY " + pts(surf));
		return 0;
	}

//	Uint32 pixel;
    int bpp = surf->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surf->pixels + y * surf->pitch + x * bpp;

    switch (bpp)
	{
	    case 1:
	        return *p;
			break;
	    case 2:
	        return *(Uint16 *)p;
			break;
	    case 3:
	        if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
	            return p[0] << 16 | p[1] << 8 | p[2];
	        else
	            return p[0] | p[1] << 8 | p[2] << 16;
			break;
	    case 4:
	        return *(Uint32 *)p;
			break;
	    default:
	        return 0; /* shouldn't happen, but avoids warnings */
	        break;
    }
}

bool SDL_SetPixel(SDL_Surface* surf, Uint32 x, Uint32 y,
					Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool copy)
{
	if (!surf)
		return false;

    Uint32 	Rmask = surf->format->Rmask,
			Gmask = surf->format->Gmask,
			Bmask = surf->format->Bmask,
			Amask = surf->format->Amask;
    Uint32 R, G, B, A = 0;

    //Constrain this function to the surfaces clip
	if (x >= surf->clip_rect.w + surf->clip_rect.x
		|| y >= surf->clip_rect.h + surf->clip_rect.y
		|| x < surf->clip_rect.x
		|| y < surf->clip_rect.y)
	{
		//WARNING("Out of Bounds " + pts(surface));
		return false;
	}

	Uint32 c = SDL_MapRGBA(surf->format, r, g, b, a);

	switch (surf->format->BytesPerPixel) {
		case 1:{		/* Assuming 8-bpp */
			if (a == 255 || copy) {
			    *((Uint8 *) surf->pixels + y * surf->pitch + x) = c;
			} else {
			    Uint8 *pixel = (Uint8 *) surf->pixels + y * surf->pitch + x;

			    Uint8 dR = surf->format->palette->colors[*pixel].r;
			    Uint8 dG = surf->format->palette->colors[*pixel].g;
			    Uint8 dB = surf->format->palette->colors[*pixel].b;
			    Uint8 sR = surf->format->palette->colors[c].r;
			    Uint8 sG = surf->format->palette->colors[c].g;
			    Uint8 sB = surf->format->palette->colors[c].b;

			    dR = dR + ((sR - dR) * a >> 8);
			    dG = dG + ((sG - dG) * a >> 8);
			    dB = dB + ((sB - dB) * a >> 8);

			    *pixel = SDL_MapRGB(surf->format, dR, dG, dB);
			}
		} break;
		case 2:{		/* Probably 15-bpp or 16-bpp */
			if (a == 255 || copy) {
			    *((Uint16 *) surf->pixels + y * surf->pitch / 2 + x) = c;
			} else {
			    Uint16 *pixel = (Uint16 *) surf->pixels + y * surf->pitch / 2 + x;
			    Uint32 dc = *pixel;

			    R = ((dc & Rmask) + (((c & Rmask) - (dc & Rmask)) * a >> 8)) & Rmask;
			    G = ((dc & Gmask) + (((c & Gmask) - (dc & Gmask)) * a >> 8)) & Gmask;
			    B = ((dc & Bmask) + (((c & Bmask) - (dc & Bmask)) * a >> 8)) & Bmask;
			    if (Amask)
				A = ((dc & Amask) + (((c & Amask) - (dc & Amask)) * a >> 8)) & Amask;

			    *pixel = R | G | B | A;
			}
		} break;
		case 3:{		/* Slow 24-bpp mode, usually not used */
			Uint8 *pix = (Uint8 *) surf->pixels + y * surf->pitch + x * 3;
			Uint8 rshift8 = surf->format->Rshift / 8;
			Uint8 gshift8 = surf->format->Gshift / 8;
			Uint8 bshift8 = surf->format->Bshift / 8;
			Uint8 ashift8 = surf->format->Ashift / 8;


			if (a == 255 || copy) {
			    *(pix + rshift8) = c >> surf->format->Rshift;
			    *(pix + gshift8) = c >> surf->format->Gshift;
			    *(pix + bshift8) = c >> surf->format->Bshift;
			    *(pix + ashift8) = c >> surf->format->Ashift;
			} else {
			    Uint8 dR, dG, dB, dA = 0;
			    Uint8 sR, sG, sB, sA = 0;

			    pix = (Uint8 *) surf->pixels + y * surf->pitch + x * 3;

			    dR = *((pix) + rshift8);
			    dG = *((pix) + gshift8);
			    dB = *((pix) + bshift8);
			    dA = *((pix) + ashift8);

			    sR = (c >> surf->format->Rshift) & 0xff;
			    sG = (c >> surf->format->Gshift) & 0xff;
			    sB = (c >> surf->format->Bshift) & 0xff;
			    sA = (c >> surf->format->Ashift) & 0xff;

			    dR = dR + ((sR - dR) * a >> 8);
			    dG = dG + ((sG - dG) * a >> 8);
			    dB = dB + ((sB - dB) * a >> 8);
			    dA = dA + ((sA - dA) * a >> 8);

			    *((pix) + rshift8) = dR;
			    *((pix) + gshift8) = dG;
			    *((pix) + bshift8) = dB;
			    *((pix) + ashift8) = dA;
			}
		} break;
		case 4: {		/* Probably 32-bpp */
			if (a == 255 || copy) {
			    *((Uint32 *) surf->pixels + y * surf->pitch / 4 + x) = c;
			} else {
			    Uint32 Rshift, Gshift, Bshift, Ashift;
			    Uint32 *pixel = (Uint32 *) surf->pixels + y * surf->pitch / 4 + x;
			    Uint32 dc = *pixel;

			    Rshift = surf->format->Rshift;
			    Gshift = surf->format->Gshift;
			    Bshift = surf->format->Bshift;
			    Ashift = surf->format->Ashift;

			    R = ((dc & Rmask) + (((((c & Rmask) - (dc & Rmask)) >> Rshift) * a >> 8) << Rshift)) & Rmask;
			    G = ((dc & Gmask) + (((((c & Gmask) - (dc & Gmask)) >> Gshift) * a >> 8) << Gshift)) & Gmask;
			    B = ((dc & Bmask) + (((((c & Bmask) - (dc & Bmask)) >> Bshift) * a >> 8) << Bshift)) & Bmask;
			    if (Amask)
				A = ((dc & Amask) + (((((c & Amask) - (dc & Amask)) >> Ashift) * a >> 8) << Ashift)) & Amask;

			    *pixel = R | G | B | A;
			}
		} break;
	}
    return true;
}


SDL_Surface* SDL_NewSurface(Uint32 width, Uint32 height, color colorKey, bool alphaChannel)
{
	SDL_Surface *blankImg = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32,
													 RMASK, GMASK, BMASK, AMASK);

    if (blankImg == NULL)
	{
        FATAL("SDL_CreateRGBSurface error");
    }

    SDL_SetColorKey(blankImg, SDL_SRCCOLORKEY | SDL_RLEACCEL,
			SDL_MapRGB(blankImg->format, colorKey.r, colorKey.g, colorKey.b));

    SDL_Surface* blankImgB;
	if (alphaChannel)
		blankImgB = SDL_DisplayFormatAlpha(blankImg);
	else //Yes. It matters. Fucking SDL.
		blankImgB = SDL_DisplayFormat(blankImg);

    SDL_FreeSurface(blankImg);

    if (!alphaChannel)
    	SDL_FillRect(blankImgB, &blankImgB->clip_rect,
					SDL_MapRGB(blankImgB->format, colorKey.r, colorKey.g, colorKey.b));

	return blankImgB;

}

/*	Creates an exact duplication of the given rect as a new SDL_Surface */
SDL_Surface* SDL_CopySurface(SDL_Surface* src, SDL_Rect r)
{
	int dy = 0;
	int y;
	int offset;
	SDL_Surface* dst = NULL;
	char* pixels = NULL;

	SDL_Rect rr;
	rr.x = rr.y = 0;
	rr.w = src->w;
	rr.h = src->h;

	//constrain clip to our image. TODO: are these calculations right?
	if (r.x < rr.x) r.x = rr.x;
	if (r.y < rr.y) r.y = rr.y;
	if (r.x + r.w > rr.w) r.w = rr.w - r.x;
	if (r.y + r.h > rr.h) r.h = rr.h - r.y;

	/*if (!ConstrainRect(r, rr))
	{
		printf("Invalid Src Rect\n");
		return NULL;
	}*/

	//to avoid having to deal with palettes and lower BPP conversions, let's just use SDL to convert the
	//image to a 4 BPP version
	SDL_Surface* tmp = SDL_DisplayFormatAlpha( src );

	if (!tmp)
		return NULL;

	//if we're copying the entire surface, SDL_DisplayFormatAlpha did it for us already.
	if (r.x == 0 && r.y == 0 && r.w == tmp->w && r.h == tmp->h)
		return tmp;

	//Otherwise, need to create a new surface and do a bit of memory work.

	//Generate a container for pixels
	dst = SDL_CreateRGBSurface(SDL_SWSURFACE,
				r.w, r.h, tmp->format->BitsPerPixel,
				tmp->format->Rmask, tmp->format->Gmask,
				tmp->format->Bmask, tmp->format->Amask);

	if (SDL_MUSTLOCK(dst))
		SDL_LockSurface(dst);
	if (SDL_MUSTLOCK(tmp))
		SDL_LockSurface(tmp);

	//Copy the subsection of pixels over
	for (y = r.y; y < r.y + r.h; y++)
	{
		memcpy((char*)dst->pixels + (dst->pitch * dy),
				(char*)tmp->pixels + (tmp->pitch * y + (r.x * tmp->format->BytesPerPixel)),
				dst->pitch);
		dy++;
	}

	if (SDL_MUSTLOCK(dst))
		SDL_UnlockSurface(dst);
	if (SDL_MUSTLOCK(tmp))
		SDL_UnlockSurface(tmp);

	SDL_FreeSurface(tmp);

	return dst;
}

void SDL_Colorize(SDL_Surface* src, Uint8 r, Uint8 g, Uint8 b)
{
	Uint8 r2, g2, b2, a2;

	LOCKSURF(src);
	for (Uint32 y = 0; y < src->h; y++)
	{
		for (Uint32 x = 0; x < src->w; x++)
		{

			SDL_GetRGBA( SDL_GetPixel(src, x, y), src->format, &r2, &g2, &b2, &a2 );

			//If greyscale, perform pixel adjustment
			if ( r2 == g2 && g2 == b2)
			{
				// TODO: Better calculation
				r2 = (Uint8)((double)r + ( (double)(255 - r2) / 255 * (-(double)r) ));
				g2 = (Uint8)((double)g + ( (double)(255 - g2) / 255 * (-(double)g) ));
				b2 = (Uint8)((double)b + ( (double)(255 - b2) / 255 * (-(double)b) ));

				SDL_SetPixel(src, x, y, r2, g2, b2, a2, true);
			}

		}
	}
	UNLOCKSURF(src);
}


